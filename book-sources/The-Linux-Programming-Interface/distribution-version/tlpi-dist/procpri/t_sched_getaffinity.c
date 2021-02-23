/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 35 */

/* t_sched_getaffinity.c

   Demonstrate the use of the sched_getaffinity() system call to retrieve
   the CPU affinity of a process.

   Usage: t_sched_getaffinity pid

   See also t_sched_setaffinity.c.

   This program is Linux-specific. The CPU affinity system calls are provided
   since kernel 2.6.
*/
#define _GNU_SOURCE
#include <sched.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    pid_t pid;
    cpu_set_t set;
    size_t s;
    int cpu;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pid\n", argv[0]);

    pid = getInt(argv[1], GN_NONNEG, "pid");

    s = sched_getaffinity(pid, sizeof(cpu_set_t), &set);
    if (s == -1)
        errExit("sched_getaffinity");

    printf("CPUs:");
    for (cpu = 0; cpu < CPU_SETSIZE; cpu++)
        if (CPU_ISSET(cpu, &set))
            printf(" %d", cpu);
    printf("\n");

    exit(EXIT_SUCCESS);
}
