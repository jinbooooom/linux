/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 52-6 */

/* mq_notify_sigwaitinfo.c

   Usage: mq_notify_sigwaitinfo mq-name

   Demonstrate message notification via signals (accepting the signals with
   sigwaitinfo()) on a POSIX message queue.
*/
#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include "tlpi_hdr.h"

#define NOTIFY_SIG SIGRTMIN     /* Signal used for message notifications */

/* This program does not handle the case where a message already exists on
   the queue by the time the first attempt is made to register for message
   notification. In that case, the program would never receive a notification.
   Compare mq_notify_sig.c and mq_notify_via_signal.c for some hints on how
   this program might be modified to handle that case. */

int
main(int argc, char *argv[])
{
    struct sigevent sev;
    mqd_t mqd;
    struct mq_attr attr;
    void *buffer;
    ssize_t numRead;
    sigset_t blockMask;
    siginfo_t si;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s mq-name\n", argv[0]);

    mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    /* Determine mq_msgsize for message queue, and allocate an input buffer
       of that size */

    if (mq_getattr(mqd, &attr) == -1)
        errExit("mq_getattr");
    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL)
        errExit("malloc");

    /* Block the signal that we'll accept using sigwaitinfo() */

    sigemptyset(&blockMask);
    sigaddset(&blockMask, NOTIFY_SIG);
    if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1)
        errExit("sigprocmask");

    /* Set up message notification using the signal NOTIFY_SIG */

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = NOTIFY_SIG;
    sev.sigev_value.sival_ptr = &mqd;
                /* This allows us to obtain a pointer to 'mqd' in the
                   siginfo_t structure returned by sigwaitinfo() */

    if (mq_notify(mqd, &sev) == -1)
        errExit("mq_notify");

    for (;;) {

        /* Wait for a signal; when it is received, display associated
           information */

        if (sigwaitinfo(&blockMask, &si) == -1)
            errExit("sigwaitinfo");

        printf("Accepted signal:\n");
        printf("        si_signo   = %d\n", si.si_signo);
        printf("        si_pid     = %ld\n", (long) si.si_pid);
        printf("        si_uid     = %ld\n", (long) si.si_uid);
        printf("        si_code    = %d (%s)\n", si.si_code,
                (si.si_code == SI_MESGQ) ? "SI_MESGQ" : "???");
        printf("        *sival_ptr = %p\n\n", si.si_value.sival_ptr);

        /* Reestablish message notification */

        if (mq_notify(mqd, &sev) == -1)
            errExit("mq_notify");

        /* Although only one signal might have been queued (if NOTIFY_SIG
           is a standard signal) we might have received multiple messages,
           so use nonblocking mq_receive() calls inside a loop to read
           as many messages as possible. */

        while ((numRead = mq_receive(mqd, buffer, attr.mq_msgsize, NULL)) >= 0)
            printf("Read %ld bytes\n", (long) numRead);
            /*FIXME: above: should use %zd here, and remove (long) cast */

        if (errno != EAGAIN)            /* Unexpected error */
            errExit("mq_receive");
    }
}
