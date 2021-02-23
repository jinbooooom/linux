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

#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/wait.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static void
usage(char *pname)
{
    fprintf(stderr, "%s [-f] /proc/PID/ns/FILE cmd [arg...]\n", pname);
    fprintf(stderr, "    -f  Do a fork() after entering the namespace\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    /* Parse command-line options. The initial '+' character in the final
       getopt(3) argument prevents GNU-style permutation of command-line
       options. Preventing that is useful, since sometimes the 'command'
       to be executed by this program itself has command-line options. We
       don't want getopt() to treat those as options to this program. */

    int do_fork = 0;
    int opt;

    while ((opt = getopt(argc, argv, "+f")) != -1) {
        switch (opt) {
        case 'f': do_fork = true;       break;
        default:  usage(argv[0]);
        }
    }

    if (argc < optind + 2)
        usage(argv[1]);

    /* Get file descriptor for namespace; the file descriptor is opened with
       O_CLOEXEC so as to ensure that it is not inherited by the program that
       is later executed. */

    int fd = open(argv[optind], O_RDONLY | O_CLOEXEC);
    if (fd == -1)
        errExit("open");

    if (setns(fd, 0) == -1)         /* Join that namespace */
        errExit("setns");

    /* If '-f' was specified, fork to create a child that is waited on by
       the parent. This is useful when entering a PID namespaces, since
       setns() into a PID namespace does not move the calling process into
       the namespace, but only changes the PID namespace in which the
       children of the caller will be created.  */

    if (do_fork) {
        pid_t pid = fork();
        if (pid == -1)
            errExit("fork");

        if (pid != 0) {         /* Parent lands here; child falls through */
            wait(NULL);
            exit(EXIT_SUCCESS);
        }
    }

    /* Execute a command in namespace */

    execvp(argv[optind + 1], &argv[optind + 1]);
    errExit("execvp");
}
