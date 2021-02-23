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

/* cpu_multi_burner.c

   Usage: cpu_multi_burner period...

   This program creates one child process per command-line argument.
   Each child loops consuming CPU, and, after each 'period' seconds
   of consumed CPU, reports its process ID, total CPU time consumed,
   and rate of CPU consumption per real second since the last report.

   For some experiments, it is useful to confine all processes to the
   same CPU, using taskset(1). For example:

        taskset 0x1 ./cpu_multi_burner 2 2

   See also cpu_multithread_burner.c.
*/
#include <sys/times.h>
#include <time.h>
#include "tlpi_hdr.h"

#define NANO 1000000000

static void
burnCPU(float period)
{
    int curr_step;      /* Current number of intervals of consumed CPU time */
    int prev_step;      /* Number of intervals of consumed CPU time calculated
                           in previous loop iteration */
    struct timespec curr_cpu;
    struct timespec curr_rt, prev_rt;
    int elapsed_rt_us;  /* Elapsed real microseconds for current CPU interval */

    prev_step = 0;

    if (clock_gettime(CLOCK_REALTIME, &prev_rt) == -1)
        errExit("clock_gettime");

    while (1) {
        if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &curr_cpu) == -1)
            errExit("clock_gettime");

        curr_step = curr_cpu.tv_sec / period + curr_cpu.tv_nsec / period / NANO;

        if (clock_gettime(CLOCK_REALTIME, &curr_rt) == -1)
            errExit("clock_gettime");

        elapsed_rt_us = (curr_rt.tv_sec - prev_rt.tv_sec) * 1000000 +
                     (curr_rt.tv_nsec - prev_rt.tv_nsec) / 1000;

        if (curr_step > prev_step) {
            printf("[%ld]  CPU: %.3f; elapsed/cpu = %0.3f; %%CPU = %.3f\n",
                    (long) getpid(),
                    (float) curr_step * period,
                    elapsed_rt_us / 1000000.0 / period,
                    period / (elapsed_rt_us / 1000000.0) * 100.0);
            prev_step = curr_step;
            prev_rt = curr_rt;
        }
    }
}

int
main(int argc, char *argv[])
{
    float period;
    int nproc;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s [period]...\n"
                "Creates one process per argument that reports "
                "CPU time each 'period' CPU seconds\n"
                "'period' can be a floating-point number\n", argv[0]);

    nproc = argc - 1;

    for (int j = 0; j < nproc; j++) {

        switch (fork()) {
        case 0:
            sscanf(argv[j + 1], "%f", &period);
            burnCPU(period);
            exit(EXIT_SUCCESS);         /* NOTREACHED */

        case -1:
            errExit("fork");

        default:
            break;
        }
    }

    pause();

    exit(EXIT_SUCCESS);
}
