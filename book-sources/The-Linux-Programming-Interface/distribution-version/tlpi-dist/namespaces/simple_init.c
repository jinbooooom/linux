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

/* simple_init.c

   A simple init(1)-style program to be used as the init program in
   a PID namespace. The program reaps the status of its children and
   provides a simple shell facility for executing commands.

   See https://lwn.net/Articles/532748/
*/
#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <wordexp.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mount.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int verbose = 0;

/* Display wait status (from waitpid() or similar) given in 'wstatus' */

/* SIGCHLD handler: reap child processes as they change state */

static void
child_handler(int sig)
{
    pid_t pid;
    int wstatus;

    /* WUNTRACED and WCONTINUED allow waitpid() to catch stopped and
       continued children (in addition to terminated children) */

    while ((pid = waitpid(-1, &wstatus,
                          WNOHANG | WUNTRACED | WCONTINUED)) != 0) {
        if (pid == -1) {
            if (errno == ECHILD)        /* No more children */
                break;
            else
                perror("waitpid");      /* Unexpected error */
        }

        if (verbose)
            printf("\tinit: SIGCHLD handler: PID %ld terminated\n",
                    (long) pid);
    }
}

/* Perform word expansion on string in 'cmd', allocating and
   returning a vector of words on success or NULL on failure */

static char **
expand_words(char *cmd)
{
    char **arg_vec;
    wordexp_t pwordexp;

    int s = wordexp(cmd, &pwordexp, 0);
    if (s != 0) {
        fprintf(stderr, "Word expansion failed.\n"
                        "\tNote that only simple "
                        "commands plus arguments are supported\n"
                        "\t(no pipelines, I/O redirection, and so on)\n");
        return NULL;
    }

    arg_vec = calloc(pwordexp.we_wordc + 1, sizeof(char *));
    if (arg_vec == NULL)
        errExit("calloc");

    for (int j = 0; j < pwordexp.we_wordc; j++)
        arg_vec[j] = pwordexp.we_wordv[j];

    arg_vec[pwordexp.we_wordc] = NULL;

    return arg_vec;
}

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [-v] [-p proc-mount]\n", pname);
    fprintf(stderr, "\t-v              Provide verbose logging\n");
    fprintf(stderr, "\t-p proc-mount   Mount a procfs at specified path\n");

    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    struct sigaction sa;
#define CMD_SIZE 10000
    char cmd[CMD_SIZE];
    pid_t pid;
    int opt;
    char *proc_path;

    proc_path = NULL;
    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
        case 'p': proc_path = optarg;   break;
        case 'v': verbose = 1;          break;
        default:  usage(argv[0]);
        }
    }

    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = child_handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");

    if (verbose)
        printf("\tinit: my PID is %ld\n", (long) getpid());

    /* Performing terminal operations while not being the foreground
       process group for the terminal generates a SIGTTOU that stops the
       process.  However our init "shell" needs to be able to perform
       such operations (just like a normal shell), so we ignore that
       signal, which allows the operations to proceed successfully. */

    signal(SIGTTOU, SIG_IGN);

    /* Become leader of a new process group and make that process
       group the foreground process group for the terminal */

    if (setpgid(0, 0) == -1)
        errExit("setpgid");;
    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
        errExit("tcsetpgrp-child");

    /* If the user asked to mount a procfs, mount it at the specified path */

    if (proc_path != NULL) {

        /* Some distributions enable mount propagation (mount --make-shared)
           by default. This would cause the mount that we create here to
           propagate to other namespaces. If we were mounting the
           procfs for this new PID namespace at "/proc" (which is typical),
           then this would hide the original "/proc" mount point in the
           initial namespace, which we probably don't want, since it will
           confuse a lot of system tools. To prevent propagation from
           occurring, we need to mark the mount point either as a slave
           mount or as a private mount.

           For further information on this topic, see the kernel source
           file Documentation/filesystems/sharedsubtree.txt and the
           mount(8) man page */

        if (verbose)
            printf("Making %s a private mount\n", proc_path);

        /* EINVAL is the case that occurs if 'proc_path' exists but is
           not (yet) a mount point */

        if (mount("none", proc_path, NULL, MS_SLAVE, NULL) == -1 &&
                errno != EINVAL)
            perror("mount-make-slave-/");

        if (verbose)
            printf("Mounting procfs at %s\n", proc_path);

        if (mount("proc", proc_path, "proc", 0, NULL) == -1)
            errExit("mount-procfs");
    }

    /* Loop executing "shell" commands. Note that our shell facility is
       very simple: it handles simple commands with arguments, and
       performs wordexp() expansions (globbing, variable and command
       substitution, tilde expansion, and quote removal). Complex
       commands (pipelines, ||, &&) and I/O redirections, and
       standard shell features are not supported. */

    while (1) {

        /* Read a shell command; exit on end of file */

        printf("init$ ");
        if (fgets(cmd, CMD_SIZE, stdin) == NULL) {
            if (verbose)
                printf("\tinit: exiting");
            printf("\n");
            break;
        }

        if (cmd[strlen(cmd) - 1] == '\n')
            cmd[strlen(cmd) - 1] = '\0';        /* Strip trailing '\n' */

        if (strlen(cmd) == 0)
            continue;           /* Ignore empty commands */

        pid = fork();           /* Create child process */
        if (pid == -1) {
            perror("fork");
            break;
        }

        if (pid == 0) {         /* Child */
            char **arg_vec;

            arg_vec = expand_words(cmd);
            if (arg_vec == NULL)        /* Word expansion failed */
                exit(EXIT_FAILURE);

            /* Make child the leader of a new process group and
               make that process group the foreground process
               group for the terminal */

            if (setpgid(0, 0) == -1)
                errExit("setpgid");;
            if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
                errExit("tcsetpgrp-child");

            /* Child executes shell command and terminates */

            execvp(arg_vec[0], arg_vec);
            errExit("execvp");          /* Only reached if execvp() fails */
        }

        /* Parent falls through to here */

        if (verbose)
            printf("\tinit: created child %ld\n", (long) pid);

        pause();                /* Will be interrupted by signal handler */

        /* After child changes state, ensure that the 'init' program
           is the foreground process group for the terminal */

        if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
            errExit("tcsetpgrp-parent");
    }

    /* If we mounted a procfs earlier, unmount it before terminating */

    if (proc_path != NULL) {
        if (verbose)
            printf("Unmounting procfs at %s\n", proc_path);
        if (umount(proc_path) == -1)
            errExit("umount-procfs");
    }

    exit(EXIT_SUCCESS);
}
