/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 23 */

/* cpu_burner.c

   A small program that simply consumes CPU time, displaying the amount of
   elapsed time that was required to consume each CPU second.
*/
#include <sys/times.h>
#include <time.h>
#include <signal.h>
#include "tlpi_hdr.h"

static volatile sig_atomic_t gotSig = 0;

static void
handler(int sig)
{
    gotSig = 1;
}

int
main(int argc, char *argv[])
{
    time_t prev_cpu_secs;
    struct timespec curr_cpu;
    struct timespec curr_rt, prev_rt;
    struct sigaction sa;
    int elapsed_us;

    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGTERM, &sa, NULL) == -1)
        errExit("sigaction");
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    prev_cpu_secs = 0;
    if (clock_gettime(CLOCK_REALTIME, &prev_rt) == -1)
        errExit("clock_gettime");

    /* Loop consuming CPU time until we get sent a signal */

    while (!gotSig) {
        if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curr_cpu) == -1)
            errExit("clock_gettime");

        /* Each time the CPU time clock ticks over to another second,
           display the amount of time that elapsed since consuming the
           previous second of CPU time. */

        if (curr_cpu.tv_sec > prev_cpu_secs) {
            if (clock_gettime(CLOCK_REALTIME, &curr_rt) == -1)
                errExit("clock_gettime");

            elapsed_us = (curr_rt.tv_sec - prev_rt.tv_sec) * 1000000 +
                         (curr_rt.tv_nsec - prev_rt.tv_nsec) / 1000;
            printf("[%ld] %ld: elapsed/cpu = %5.3f; %%CPU = %5.3f\n",
                    (long) getpid(),
                    (long) curr_cpu.tv_sec, elapsed_us / 1000000.0,
                    1000000.0 / elapsed_us * 100.0);
            prev_cpu_secs = curr_cpu.tv_sec;
            prev_rt = curr_rt;
        }
    }

    printf("Bye!\n");
    exit(EXIT_SUCCESS);
}
