/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 53 */

#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include "tlpi_hdr.h"

#define NOTIFY_SIG SIGUSR1

static volatile sig_atomic_t gotSig = 1;

/* Handler for message notification signal */

static void
handler(int sig, siginfo_t *si, void *ucontext)
{
    gotSig = 1;
    printf("Signaled: si_pid = %ld\n", (long) si->si_pid);
                /* UNSAFE (see Section 21.1.2) */
}

int
main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s /mq-name\n", argv[0]);

    mqd_t mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    if (sigaction(NOTIFY_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    for (int j = 0; ; j++) {
        if (gotSig) {
            struct sigevent sev;
            const int MAX_MSG_SIZE = 8192;
            char msg[MAX_MSG_SIZE];

            gotSig = 0;

            sev.sigev_notify = SIGEV_SIGNAL;
            sev.sigev_signo = NOTIFY_SIG;
            if (mq_notify(mqd, &sev) == -1)
                errExit("mq_notify");

            ssize_t numRead;
            while ((numRead = mq_receive(mqd, msg, MAX_MSG_SIZE,
                                         NULL)) >= 0) {

                printf("Read %zd bytes\n", numRead);
            }
            if (errno != EAGAIN)        /* Unexpected error */
                errExit("mq_receive");
        }

        printf("j = %d\n", j);
        sleep(5);               /* Do some "work" */
    }
}
