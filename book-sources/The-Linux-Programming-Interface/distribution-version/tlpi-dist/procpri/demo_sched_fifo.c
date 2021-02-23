/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 35-3 */

/* demo_sched_fifo.c

   This program demonstrates the use of realtime scheduling policies. It creates
   two processes, each running under the SCHED_FIFO scheduling policy. Each
   process executes a function that prints a message every quarter of a second
   of CPU time. After each second of consumed CPU time, the function and calls
   sched_yield() to yield the CPU to the other process. Once a process has
   consumed 3 seconds of CPU time, the function terminates.

   This program must be run as superuser, or (on Linux 2.6.12 and later)
   with a suitable RLIMIT_RTPRIO resource limit.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <sys/resource.h>
#include <sys/times.h>
#include "tlpi_hdr.h"

#define CSEC_STEP 25            /* CPU centiseconds between messages */

static void
useCPU(char *msg)
{
    struct tms tms;
    int cpuCentisecs, prevStep, prevSec;

    prevStep = 0;
    prevSec = 0;
    for (;;) {
        if (times(&tms) == -1)
            errExit("times");
        cpuCentisecs = (tms.tms_utime + tms.tms_stime) * 100 /
                        sysconf(_SC_CLK_TCK);

        if (cpuCentisecs >= prevStep + CSEC_STEP) {
            prevStep += CSEC_STEP;
            printf("%s (PID %ld) cpu=%0.2f\n", msg, (long) getpid(),
                    cpuCentisecs / 100.0);
        }

        if (cpuCentisecs > 300)         /* Terminate after 3 seconds */
            break;

        if (cpuCentisecs >= prevSec + 100) {    /* Yield once/second */
            prevSec = cpuCentisecs;
            sched_yield();
        }
    }
}

int
main(int argc, char *argv[])
{
    struct rlimit rlim;
    struct sched_param sp;
    cpu_set_t set;

    setbuf(stdout, NULL);               /* Disable buffering of stdout */

    /* Confine all processes to a single CPU, so that the processes
       won't run in parallel on multi-CPU systems. */

    CPU_ZERO(&set);
    CPU_SET(1, &set);

    if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
        errExit("sched_setaffinity");

    /* Establish a CPU time limit. This demonstrates how we can
       ensure that a runaway realtime process is terminated if we
       make a programming error. The resource limit is inherited
       by the child created using fork().

       An alternative technique would be to make an alarm() call in each
       process (since interval timers are not inherited across fork()). */

    rlim.rlim_cur = rlim.rlim_max = 50;
    if (setrlimit(RLIMIT_CPU, &rlim) == -1)
        errExit("setrlimit");

    /* Run the two processes in the lowest SCHED_FIFO priority */

    sp.sched_priority = sched_get_priority_min(SCHED_FIFO);
    if (sp.sched_priority == -1)
        errExit("sched_get_priority_min");

    if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
        errExit("sched_setscheduler");

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0:
        useCPU("child ");
        exit(EXIT_SUCCESS);

    default:
        useCPU("parent");
        exit(EXIT_SUCCESS);
    }
}
