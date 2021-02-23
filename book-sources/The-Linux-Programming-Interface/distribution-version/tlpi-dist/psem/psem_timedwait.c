/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 53-2 */

/* psem_timedwait.c

   Decrease the value of a POSIX named semaphore using sem_timedwait().

   Usage: psem_timedwait sem-name nsecs

   On Linux, named semaphores are supported with kernel 2.6 or later, and
   a glibc that provides the NPTL threading implementation.
*/
#define _POSIX_C_SOURCE 199309
#include <semaphore.h>
#include <time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    sem_t *sem;
    struct timespec ts;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s sem-name num-secs\n", argv[0]);

    sem = sem_open(argv[1], 0);
    if (sem == SEM_FAILED)
        errExit("sem_open");

    /* sem_timedwait() expects an absolute time in its second argument.
       So we take the number of (relative) seconds specified on the
       command line, and add it to the current system time. */

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        errExit("clock_gettime-CLOCK_REALTIME");

    ts.tv_sec += atoi(argv[2]);

    if (sem_timedwait(sem, &ts) == -1)
        errExit("sem_timedwait");

    printf("%ld sem_wait() succeeded\n", (long) getpid());
    exit(EXIT_SUCCESS);
}
