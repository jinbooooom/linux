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

/* unshare.c

   A simple implementation of the unshare(1) command: unshare
   namespaces and execute a command.

   See https://lwn.net/Articles/531381/
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

#ifndef CLONE_NEWCGROUP         /* Added in Linux 4.6 */
#define CLONE_NEWCGROUP         0x02000000
#endif

#ifndef CLONE_NEWTIME           /* Added in Linux 5.6 */
#define CLONE_NEWTIME           0x00000080
#endif

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] cmd [arg...]\n", pname);
    fprintf(stderr, "Options can be:\n");
    fprintf(stderr, "    -f   fork() before executing cmd "
            "(useful when unsharing PID namespace)\n");
    fprintf(stderr, "    -C   unshare cgroup namespace\n");
    fprintf(stderr, "    -i   unshare IPC namespace\n");
    fprintf(stderr, "    -m   unshare mount namespace\n");
    fprintf(stderr, "    -n   unshare network namespace\n");
    fprintf(stderr, "    -p   unshare PID namespace\n");
    fprintf(stderr, "    -T   unshare time namespace\n");
    fprintf(stderr, "    -u   unshare UTS namespace\n");
    fprintf(stderr, "    -U   unshare user namespace\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int flags, do_fork, opt;

    flags = 0;
    do_fork = 0;
    while ((opt = getopt(argc, argv, "CfimnpTuU")) != -1) {
        switch (opt) {
        case 'f': do_fork = 1;                  break;
        case 'C': flags |= CLONE_NEWCGROUP;     break;
        case 'i': flags |= CLONE_NEWIPC;        break;
        case 'm': flags |= CLONE_NEWNS;         break;
        case 'n': flags |= CLONE_NEWNET;        break;
        case 'p': flags |= CLONE_NEWPID;        break;
        case 'T': flags |= CLONE_NEWTIME;       break;
        case 'u': flags |= CLONE_NEWUTS;        break;
        case 'U': flags |= CLONE_NEWUSER;       break;
        default:  usage(argv[0]);
        }
    }

    if (optind >= argc)
        usage(argv[0]);

    if (unshare(flags) == -1)
        errExit("unshare");

    /* If we are unsharing the PID namespace, then the caller is *not*
       moved into the new namespace. Instead, only the children are moved
       into the namespace. Therefore, we support an option that causes
       the program to call fork() before executing the specified program,
       in order to create a new child that will be created in a new PID
       namespace. */

    if (do_fork) {
        if (fork()) {
            wait(NULL);         /* Parent waits for child to complete */
            exit(EXIT_SUCCESS);
        }

        /* Child falls through to execute command */
    }

    execvp(argv[optind], &argv[optind]);
    errExit("execvp");
}
