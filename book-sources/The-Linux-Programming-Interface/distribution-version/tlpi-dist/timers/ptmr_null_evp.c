/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 23-3 */

/* ptmr_null_evp.c

   A program to demonstrate POSIX timer defaults when the 'evp' argument
   of timer_create() is specified as NULL.

   Kernel support for Linux timers is provided since Linux 2.6. On older
   systems, an incomplete user-space implementation of POSIX timers
   was provided in glibc.
*/
#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include <time.h>
#include "curr_time.h"          /* Declaration of currTime() */
#include "tlpi_hdr.h"

static void
handler(int sig, siginfo_t *si, void *uc)
{
    printf("[%s] Got signal %d\n", currTime("%T"), sig);
    printf("    sival_int          = %d\n", si->si_value.sival_int);
#ifdef __linux__
    printf("    si_overrun         = %d\n", si->si_overrun);
#endif
    printf("    timer_getoverrun() = %d\n",
            timer_getoverrun((timer_t) si->si_value.sival_ptr));
}

int
main(int argc, char *argv[])
{
    struct itimerspec ts;
    timer_t tid;
    int j;
    struct sigaction sa;

    if (argc < 2)
        usageErr("%s secs [nsecs [int-secs [int-nsecs]]]\n", argv[0]);

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1)
        errExit("sigaction");

    if (timer_create(CLOCK_REALTIME, NULL, &tid) == -1)
        errExit("timer_create");
    printf("timer ID = %ld\n", (long) tid);

    ts.it_value.tv_sec = atoi(argv[1]);
    ts.it_value.tv_nsec = (argc > 2) ? atoi(argv[2]) : 0;
    ts.it_interval.tv_sec = (argc > 3) ? atoi(argv[3]) : 0;
    ts.it_interval.tv_nsec = (argc > 4) ? atoi(argv[4]) : 0;
    if (timer_settime(tid, 0, &ts, NULL) == -1)
        errExit("timer_settime");

    for (j = 0; ; j++)
        pause();
}
