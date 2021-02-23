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

/* demo_userns.c

   Demonstrate the use of the clone() CLONE_NEWUSER flag.

   Link with "-lcap" and make sure that the "libcap-devel" (or
   similar) package is installed on the system.

   See https://lwn.net/Articles/532593/
*/
#define _GNU_SOURCE
#include <sys/capability.h>
#include <sys/wait.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int                      /* Startup function for cloned child */
childFunc(void *arg)
{
    cap_t caps;
    char *str;

    for (;;) {
        printf("eUID = %ld; eGID = %ld; ",
                (long) geteuid(), (long) getegid());

        caps = cap_get_proc();
        if (caps == NULL)
            errExit("cap_get_proc");

        str = cap_to_text(caps, NULL);
        if (str == NULL)
            errExit("cap_to_text");

        printf("capabilities: %s\n", str);

        cap_free(caps);
        cap_free(str);

        if (arg == NULL)
            break;

        sleep(5);
    }

    return 0;
}

#define STACK_SIZE (1024 * 1024)

int
main(int argc, char *argv[])
{
    pid_t pid;
    char *stack;

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");

    /* Create child; child commences execution in childFunc() */

    pid = clone(childFunc,
                stack + STACK_SIZE,     /* Assume stack grows downward */
                CLONE_NEWUSER | SIGCHLD, argv[1]);
    if (pid == -1)
        errExit("clone");

    printf("PID of child: %ld\n", (long) pid);

    /* Parent falls through to here.  Wait for child. */

    if (waitpid(pid, NULL, 0) == -1)
        errExit("waitpid");

    exit(EXIT_SUCCESS);
}
