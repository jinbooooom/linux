/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 23-2 */

/* t_clock_nanosleep.c

   Demonstrate the use of clock_nanosleep() to sleep for an interval
   specified in nanoseconds.

   See also t_nanosleep.c.

   Linux supports clock_nanosleep() since kernel 2.6.
*/
#if ! defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 600
#define _XOPEN_SOURCE 600
#endif
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "tlpi_hdr.h"

static void
sigintHandler(int sig)
{
    return;                             /* Just interrupt clock_nanosleep() */
}

int
main(int argc, char *argv[])
{
    struct timeval start, finish;
    struct timespec request, remain;
    struct sigaction sa;
    int s, flags;

    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s secs nanosecs [a]\n", argv[0]);

    /* Allow SIGINT handler to interrupt clock_nanosleep() */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    /* If more than three command-line arguments, use TIMER_ABSTIME flag */

    flags = (argc > 3) ? TIMER_ABSTIME : 0;

    if (flags == TIMER_ABSTIME) {
        if (clock_gettime(CLOCK_REALTIME, &request) == -1)
            errExit("clock_gettime");
        printf("Initial CLOCK_REALTIME value: %ld.%09ld\n",
                (long) request.tv_sec, request.tv_nsec);

        request.tv_sec  += getLong(argv[1], 0, "secs");
        request.tv_nsec += getLong(argv[2], 0, "nanosecs");
        if (request.tv_nsec >= 1000000000) {
            request.tv_sec += request.tv_nsec / 1000000000;
            request.tv_nsec %= 1000000000;
        }

    } else {                    /* Relative sleep */
        request.tv_sec  = getLong(argv[1], 0, "secs");
        request.tv_nsec = getLong(argv[2], 0, "nanosecs");
    }

    if (gettimeofday(&start, NULL) == -1)
        errExit("gettimeofday");

    for (;;) {
        s = clock_nanosleep(CLOCK_REALTIME, flags, &request, &remain);
        if (s != 0 && s != EINTR)
            errExitEN(s, "clock_nanosleep");

        if (s == EINTR)
            printf("Interrupted... ");

        if (gettimeofday(&finish, NULL) == -1)
            errExit("gettimeofday");
        printf("Slept: %.6f secs", finish.tv_sec - start.tv_sec +
                        (finish.tv_usec - start.tv_usec) / 1000000.0);

        if (s == 0)
            break;                      /* sleep completed */

        if (flags != TIMER_ABSTIME) {
            printf("... Remaining: %ld.%09ld",
                    (long) remain.tv_sec, remain.tv_nsec);

            request = remain;
        }

        printf("... Restarting\n");
    }

    printf("\nSleep complete\n");
    exit(EXIT_SUCCESS);
}
