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

/* svsem_rm.c

   Remove the System V semaphore sets whose IDs are specified by the
   command-line arguments.
*/
#include <sys/types.h>
#include <sys/sem.h>
#include "semun.h"              /* Definition of semun union */
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int j;
    union semun dummy;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [semid...]\n", argv[0]);

    for (j = 1; j < argc; j++)
        if (semctl(getInt(argv[j], 0, "semid"), 0, IPC_RMID, dummy) == -1)
            errExit("semctl %s", argv[j]);

    exit(EXIT_SUCCESS);
}
