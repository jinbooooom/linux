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

/* mq_notify_siginfo.c

   Usage: mq_notify_siginfo /mq-name

   Demonstrate message notification via signals (catching the signals with
   a signal handler) on a POSIX message queue.

   See also mq_notify_sigwaitinfo.c.
*/
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include "tlpi_hdr.h"

#define NOTIFY_SIG SIGUSR1

static volatile sig_atomic_t gotSig = 1;        /* See comment in main() */

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

    /* Open the (existing) queue in nonblocking mode so that we can drain
       messages from it without blocking once the queue has been emptied */

    mqd_t mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    /* Establish handler for notification signal */

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    if (sigaction(NOTIFY_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    /* Possibly, a message had already been queued by the time we enter
       the loop below. By initializing 'gotSig' to 1 above, we trigger the
       program to make the initial registration for notification and force
       the queue to be drained of any messages on the first loop iteration. */

    for (int j = 0; ; j++) {
        if (gotSig) {
            struct sigevent sev;
            const int MAX_MSG_SIZE = 8192;
            char msg[MAX_MSG_SIZE];

            gotSig = 0;

            /* Register for message notification */

            sev.sigev_notify = SIGEV_SIGNAL;
            sev.sigev_signo = NOTIFY_SIG;
            if (mq_notify(mqd, &sev) == -1)
                errExit("mq_notify");

            /* Drain all messages from the queue */

            ssize_t numRead;
            while ((numRead = mq_receive(mqd, msg, MAX_MSG_SIZE,
                                         NULL)) >= 0) {
                /* Do whatever processing is required for each message */

                printf("Read %zd bytes\n", numRead);
            }
            if (errno != EAGAIN)        /* Unexpected error */
                errExit("mq_receive");
        }

        printf("j = %d\n", j);
        sleep(5);               /* Do some "work" */
    }
}
