/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 28 */

/* demo_clone.c

   Demonstrate the use of the Linux-specific clone() system call.

   This program creates a child using clone(). Various flags can be included
   in the clone() call by specifying option letters in the first command-line
   argument to the program (see usageError() below for a list of the option
   letters). Note that not all combinations of flags are valid. See Section
   28.2.1 or the clone(2) man page about information about which flag
   combinations are required or invalid.

   This program is Linux-specific.
*/
#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include "print_wait_status.h"
#include "tlpi_hdr.h"

#ifndef CHILD_SIG
#define CHILD_SIG SIGUSR1       /* Signal to be generated on termination
                                   of cloned child */
#endif

typedef struct {        /* For passing info to child startup function */
    int    fd;
    mode_t umask;
    int    exitStatus;
    int    signal;
} ChildParams;

static int              /* Startup function for cloned child */
childFunc(void *arg)
{
    ChildParams *cp;

    printf("Child:  PID=%ld PPID=%ld\n", (long) getpid(), (long) getppid());

    cp = (ChildParams *) arg;   /* Cast arg to true form */

    /* The following changes may affect parent */

    umask(cp->umask);
    if (close(cp->fd) == -1)
        errExit("child:close");
    if (signal(cp->signal, SIG_DFL) == SIG_ERR)
        errExit("child:signal");

    return cp->exitStatus;      /* Child terminates now */
}

static void             /* Handler for child termination signal */
grimReaper(int sig)
{
    int savedErrno;

    /* UNSAFE: This handler uses non-async-signal-safe functions
       (printf(), strsignal(); see Section 21.1.2) */

    savedErrno = errno;         /* In case we change 'errno' here */

    printf("Caught signal %d (%s)\n", sig, strsignal(sig));

    errno = savedErrno;
}

static void
usageError(char *progName)
{
    fprintf(stderr, "Usage: %s [arg]\n", progName);
#define fpe(str) fprintf(stderr, "        " str)
    fpe("'arg' can contain the following letters:\n");
    fpe("    d - share file descriptors (CLONE_FILES)\n");
    fpe("    f - share file-system information (CLONE_FS)\n");
    fpe("    s - share signal dispositions (CLONE_SIGHAND)\n");
    fpe("    v - share virtual memory (CLONE_VM)\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    const int STACK_SIZE = 65536;       /* Stack size for cloned child */
    char *stack;                        /* Start of stack buffer area */
    char *stackTop;                     /* End of stack buffer area */
    int flags;                          /* Flags for cloning child */
    ChildParams cp;                     /* Passed to child function */
    const mode_t START_UMASK = S_IWOTH; /* Initial umask setting */
    struct sigaction sa;
    char *p;
    int status;
    ssize_t s;
    pid_t pid;

    printf("Parent: PID=%ld PPID=%ld\n", (long) getpid(), (long) getppid());

    /* Set up an argument structure to be passed to cloned child, and
       set some process attributes that will be modified by child */

    cp.exitStatus = 22;                 /* Child will exit with this status */

    umask(START_UMASK);                 /* Initialize umask to some value */
    cp.umask = S_IWGRP;                 /* Child sets umask to this value */

    cp.fd = open("/dev/null", O_RDWR);  /* Child will close this fd */
    if (cp.fd == -1)
        errExit("open");

    cp.signal = SIGTERM;                /* Child will change disposition */
    if (signal(cp.signal, SIG_IGN) == SIG_ERR)  errExit("signal");

    /* Initialize clone flags using command-line argument (if supplied) */

    flags = 0;
    if (argc > 1) {
        for (p = argv[1]; *p != '\0'; p++) {
            if      (*p == 'd') flags |= CLONE_FILES;
            else if (*p == 'f') flags |= CLONE_FS;
            else if (*p == 's') flags |= CLONE_SIGHAND;
            else if (*p == 'v') flags |= CLONE_VM;
            else usageError(argv[0]);
        }
    }

    /* Allocate stack for child */

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");

    stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

    /* Establish handler to catch child termination signal */

    if (CHILD_SIG != 0) {
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = grimReaper;
        if (sigaction(CHILD_SIG, &sa, NULL) == -1)
            errExit("sigaction");
    }

    /* Create child; child commences execution in childFunc() */

    if (clone(childFunc, stackTop, flags | CHILD_SIG, &cp) == -1)
        errExit("clone");

    /* Parent falls through to here. Wait for child; __WCLONE option is
       required for child notifying with signal other than SIGCHLD. */

    pid = waitpid(-1, &status, (CHILD_SIG != SIGCHLD) ? __WCLONE : 0);
    if (pid == -1)
        errExit("waitpid");

    printf("    Child PID=%ld\n", (long) pid);
    printWaitStatus("    Status: ", status);

    /* Check whether changes made by cloned child have affected parent */

    printf("Parent - checking process attributes:\n");
    if (umask(0) != START_UMASK)
        printf("    umask has changed\n");
    else
        printf("    umask has not changed\n");

    s = write(cp.fd, "Hello world\n", 12);
    if (s == -1 && errno == EBADF)
        printf("    file descriptor %d has been closed\n", cp.fd);
    else if (s == -1)
        printf("    write() on file descriptor %d failed (%s)\n",
                cp.fd, strerror(errno));
    else
        printf("    write() on file descriptor %d succeeded\n", cp.fd);

    if (sigaction(cp.signal, NULL, &sa) == -1)
        errExit("sigaction");
    if (sa.sa_handler != SIG_IGN)
        printf("    signal disposition has changed\n");
    else
        printf("    signal disposition has not changed\n");

    exit(EXIT_SUCCESS);
}
