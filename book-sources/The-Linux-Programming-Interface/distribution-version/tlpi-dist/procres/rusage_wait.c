/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 36-1 */

/* rusage_wait.c

   Show that getrusage() RUSAGE_CHILDREN retrieves information
   only about children that have been waited on.
*/
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/resource.h>
#include "tlpi_hdr.h"

#define NSECS 3         /* Amount of CPU to consume in child */

#define SIG SIGUSR1     /* Child uses this signal to tell parent
                           that it is about to terminate */

static void
handler(int sig)
{
    /* Do nothing: just interrupt sigsuspend() */
}

static void
printChildRusage(const char *msg)
{
    struct rusage ru;

    printf("%s", msg);
    if (getrusage(RUSAGE_CHILDREN, &ru) == -1)
        errExit("getrusage");
    printf("user CPU=%.2f secs; system CPU=%.2f secs\n",
            ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0,
            ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0);
}

int
main(int argc, char *argv[])
{
    clock_t start;
    sigset_t mask;
    struct sigaction sa;

    setbuf(stdout, NULL);       /* Disable buffering of stdout */

    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1)
        errExit("sigaction");

    /* Child informs parent of impending termination using a signal;
       block that signal until the parent is ready to catch it. */

    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        errExit("sigprocmask");

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0:             /* Child */
        for (start = clock(); clock() - start < NSECS * CLOCKS_PER_SEC;)
            continue;           /* Burn NSECS seconds of CPU time */
        printf("Child terminating\n");

        /* Tell parent we're nearly done */

        if (kill(getppid(), SIG) == -1)
            errExit("kill");
        _exit(EXIT_SUCCESS);

    default:    /* Parent */
        sigemptyset(&mask);
        sigsuspend(&mask);      /* Wait for signal from child */

        sleep(2);               /* Allow child a bit more time to terminate */

        printChildRusage("Before wait: ");
        if (wait(NULL) == -1)
            errExit("wait");
        printChildRusage("After wait:  ");
        exit(EXIT_SUCCESS);
    }
}
