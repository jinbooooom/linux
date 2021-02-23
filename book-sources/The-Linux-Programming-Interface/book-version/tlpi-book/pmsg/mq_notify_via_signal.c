/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 52 */

#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include "tlpi_hdr.h"

#define NOTIFY_SIG SIGUSR1

static volatile sig_atomic_t gotSig = 1;

/* Handler for message notification signal */

static void
handler(int sig)
{
    gotSig = 1;
}

int
main(int argc, char *argv[])
{
    struct sigevent sev;
    mqd_t mqd;
    struct sigaction sa;
    char *msg;
    ssize_t numRead;
    struct mq_attr attr;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s /mq-name\n", argv[0]);

    mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(NOTIFY_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    if (mq_getattr(mqd, &attr) == -1)
        errExit("mq_getattr");

    msg = malloc(attr.mq_msgsize);
    if (msg == NULL)
        errExit("malloc");

    for (int j = 0; ; j++) {
        if (gotSig) {
            gotSig = 0;

            sev.sigev_notify = SIGEV_SIGNAL;
            sev.sigev_signo = NOTIFY_SIG;
            if (mq_notify(mqd, &sev) == -1)
                errExit("mq_notify");

            while ((numRead = mq_receive(mqd, msg,
                                attr.mq_msgsize, NULL)) >= 0) {

                printf("Read %zd bytes\n", numRead);
            }
            if (errno != EAGAIN)        /* Unexpected error */
                errExit("mq_receive");
        }

        printf("j = %d\n", j);
        sleep(5);               /* Do some "work" */
    }
}
