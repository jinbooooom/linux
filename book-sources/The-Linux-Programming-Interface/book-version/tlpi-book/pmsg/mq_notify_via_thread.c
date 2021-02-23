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

#include <pthread.h>
#include <mqueue.h>
#include <signal.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include "tlpi_hdr.h"

static void notifySetup(mqd_t *mqdp);

static void
drainQueue(mqd_t mqd)
{
    ssize_t numRead;
    char *msg;
    struct mq_attr attr;

    if (mq_getattr(mqd, &attr) == -1)
        errExit("mq_getattr");

    msg = malloc(attr.mq_msgsize);
    if (msg == NULL)
        errExit("malloc");

    while ((numRead = mq_receive(mqd, msg, attr.mq_msgsize, NULL)) >= 0) {

        printf("Read %zd bytes\n", numRead);
    }

    if (errno != EAGAIN)
        errExit("mq_receive");

    free(msg);
}

static void
threadFunc(union sigval sv)
{
    mqd_t *mqdp;

    mqdp = sv.sival_ptr;

    notifySetup(mqdp);
    drainQueue(*mqdp);
}

static void
notifySetup(mqd_t *mqdp)
{
    struct sigevent sev;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = threadFunc;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_value.sival_ptr = mqdp;

    if (mq_notify(*mqdp, &sev) == -1)
        errExit("mq_notify");
}

int
main(int argc, char *argv[])
{
    mqd_t mqd;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s /mq-name\n", argv[0]);

    mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    notifySetup(&mqd);
    drainQueue(mqd);

    pause();
}
