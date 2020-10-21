/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 22 */

#include <signal.h>
#include "tlpi_hdr.h"

static void
handler(int sig)
{
}

#define TESTSIG SIGUSR1

int
main(int argc, char *argv[])
{
    int numSigs, scnt;
    pid_t childPid;
    sigset_t blockedMask, emptyMask;
    struct sigaction sa;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num-sigs\n", argv[0]);

    numSigs = getInt(argv[1], GN_GT_0, "num-sigs");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(TESTSIG, &sa, NULL) == -1)
        errExit("sigaction");

    /* Block the signal before fork(), so that the child doesn't manage
       to send it to the parent before the parent is ready to catch it */

    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, TESTSIG);
    if (sigprocmask(SIG_SETMASK, &blockedMask, NULL) == -1)
        errExit("sigprocmask");

    sigemptyset(&emptyMask);

    switch (childPid = fork()) {
    case -1: errExit("fork");

    case 0:     /* child */
        for (scnt = 0; scnt < numSigs; scnt++) {
            if (kill(getppid(), TESTSIG) == -1)
                errExit("kill");
            if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
                    errExit("sigsuspend");
        }
        exit(EXIT_SUCCESS);

    default: /* parent */
        for (scnt = 0; scnt < numSigs; scnt++) {
            if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
                    errExit("sigsuspend");
            if (kill(childPid, TESTSIG) == -1)
                errExit("kill");
        }
        exit(EXIT_SUCCESS);
    }
}
