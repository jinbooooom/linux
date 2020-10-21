/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter Z */

/* userns_setns_test.c

   Open a /proc/PID/ns/user namespace file specified on the command
   line, and then create a child process in a new user namespace.
   Both processes then try to setns() into the namespace identified
   on the command line.  The setns() system call requires
   CAP_SYS_ADMIN in the target namespace.

   See https://lwn.net/Articles/540087/
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/capability.h>
#include <sys/mman.h>
#include "userns_functions.h"

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static void
display_symlink(char *pname, char *link)
{
    char path[PATH_MAX];
    ssize_t s;

    s = readlink(link, path, PATH_MAX);
    if (s == -1)
        errExit("readlink");

    printf("%s%s ==> %.*s\n", pname, link, (int) s, path);
}

/* Try to join the user namespace identified by the file
   descriptor 'fd'. 'pname' is a per-process string that
   the caller can use to distinguish information messages
   displayed by this function */

static void
test_setns(char *pname, int fd)
{

    display_symlink(pname, "/proc/self/ns/user");

    /* Attempt to join the user namespace specified by 'fd' */

    if (setns(fd, CLONE_NEWUSER) == -1)
        printf("%ssetns() failed: %s\n", pname, strerror(errno));
    else {
        printf("%ssetns() succeeded\n", pname);
        display_symlink(pname, "/proc/self/ns/user");
        display_creds_and_caps(pname);
    }
}

static int              /* Start function for cloned child */
childFunc(void *arg)
{
    long fd = (long) arg;

    usleep(100000);     /* Avoid intermingling with parent's output */

    /* Test whether setns() is possible from the child user namespace */

    test_setns("child: ", fd);

    return 0;
}

#define STACK_SIZE (1024 * 1024)

int
main(int argc, char *argv[])
{
    pid_t child_pid;
    long fd;
    char *stack;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s /proc/PID/ns/user\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open user namespace file specified on command line */

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        errExit("open");

    /* Create child process in new user namespace */

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");

    child_pid = clone(childFunc,
                      stack + STACK_SIZE, /* Assume stack grows downward */
                      CLONE_NEWUSER | SIGCHLD, (void *) fd);
    if (child_pid == -1)
        errExit("clone");

    /* Test whether setns() is possible from the parent user namespace */

    test_setns("parent: ", fd);
    printf("\n");

    if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    exit(EXIT_SUCCESS);
}
