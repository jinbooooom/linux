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

/* ns_run.c

   Join one or more namespaces using setns() and execute a command in
   those namespaces, possibly inside a child process.

   This program is similar in concept to nsenter(1), but has a
   different command-line interface.

   See https://lwn.net/Articles/532748/
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [-f] [-n /proc/PID/ns/FILE] cmd [arg...]\n",
            pname);
    fprintf(stderr, "\t-f     Execute command in child process\n");
    fprintf(stderr, "\t-n     Join specified namespace\n");

    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int fd, opt, do_fork;
    pid_t pid;

    /* Parse command-line options. The initial '+' character in
       the final getopt() argument prevents GNU-style permutation
       of command-line options. That's useful, since sometimes
       the 'command' to be executed by this program itself
       has command-line options. We don't want getopt() to treat
       those as options to this program. */

    do_fork = 0;
    while ((opt = getopt(argc, argv, "+fn:")) != -1) {
        switch (opt) {

        case 'n':       /* Join a namespace */

            fd = open(optarg, O_RDONLY | O_CLOEXEC);  /* Get FD for namespace */
            if (fd == -1)
                errExit("open");

            if (setns(fd, 0) == -1)      /* Join that namespace */
                errExit("setns");

            close(fd);

            break;

        case 'f':
            do_fork = 1;
            break;

        default:
            usage(argv[0]);
        }
    }

    if (argc <= optind)
        usage(argv[0]);

    /* If the "-f" option was specified, execute the supplied command
       in a child process. This is mainly useful when working with PID
       namespaces, since setns() to a PID namespace only places
       (subsequently created) child processes in the names, and
       does not affect the PID namespace membership of the caller. */

    if (do_fork) {
        pid = fork();
        if (pid == -1)
            errExit("fork");

        if (pid != 0) {                 /* Parent */
            if (waitpid(pid, NULL, 0) == -1)    /* Wait for child */
                errExit("waitpid");
            exit(EXIT_SUCCESS);
        }

        /* Child falls through to code below */
    }

    execvp(argv[optind], &argv[optind]);
    errExit("execvp");
}
