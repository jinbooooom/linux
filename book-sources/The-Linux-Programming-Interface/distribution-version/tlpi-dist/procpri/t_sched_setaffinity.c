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

/* t_sched_setaffinity.c

   Usage: t_sched_setaffinity pid mask

   Set the CPU affinity of the process identified by 'pid' according
   to the given 'mask'. For example, the following specifies that the
   given process should run on any but the first CPU of a 4-CPU system:

        t_sched_setaffinity <pid> 0xe

                (0xe = 1110 binary)

   See also t_sched_getaffinity.c.

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
    unsigned long mask;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pid mask\n", argv[0]);

    pid = getInt(argv[1], GN_NONNEG, "pid");
    mask = getLong(argv[2], GN_ANY_BASE, "octal-mask");

    CPU_ZERO(&set);

    for (int cpu = 0; mask > 0; cpu++, mask >>= 1)
        if (mask & 1)
            CPU_SET(cpu, &set);

    if (sched_setaffinity(pid, sizeof(set), &set) == -1)
        errExit("sched_setaffinity");

    exit(EXIT_SUCCESS);
}
