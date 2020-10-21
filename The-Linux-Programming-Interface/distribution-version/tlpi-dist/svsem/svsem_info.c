/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 47 */

/* svsem_info.c

   Demonstrate the use of the SEM_INFO operation to retrieve a 'seminfo'
   structure containing the current usage of System V semaphore resources.

   This program is Linux-specific.
*/
#define _GNU_SOURCE
#include <sys/sem.h>
#include "semun.h"              /* Definition of semun union */
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct seminfo info;
    union semun arg;
    int s;

    arg.__buf = &info;

    s = semctl(0, 0, SEM_INFO, arg);
    if (s == -1)
        errExit("semctl");

    printf("Maximum ID index = %d\n", s);
    printf("sets in_use      = %ld\n", (long) info.semusz);
    printf("used_sems        = %ld\n", (long) info.semaem);
    exit(EXIT_SUCCESS);
}
