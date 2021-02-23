/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Solution for Exercise 47-5:a */

/* event_flags.c

   Implement an event flags protocol using System V semaphores.

   See event_flags.h for a summary of the interface.
*/
#include <sys/types.h>
#include <sys/sem.h>
#include "semun.h"              /* Definition of semun union */
#include "event_flags.h"
#include "tlpi_hdr.h"

/* Wait for the specified flag to become "set" (0) */

int
waitForEventFlag(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 0;                    /* Wait for semaphore to equal 0 */
    sops.sem_flg = 0;

    /* Waiting for a semaphore to become zero may block, so we
       program to retry if interrupted by a signal handler */

    while (semop(semId, &sops, 1) == -1)
        if (errno != EINTR)
            return -1;
    return 0;
}

/* "Clear" the event flag (give it the value 1) */

int
clearEventFlag(int semId, int semNum)
{
    union semun arg;

    arg.val = 1;
    return semctl(semId, semNum, SETVAL, arg);
}

/* "Set" the event flag (give it the value 0) */

int
setEventFlag(int semId, int semNum)
{
    union semun arg;

    arg.val = 0;
    return semctl(semId, semNum, SETVAL, arg);
}

/* Get current state of event flag */

int
getFlagState(int semId, int semNum, Boolean *isSet)
{
    int sem_val;
    union semun dummy;

    sem_val = semctl(semId, semNum, GETVAL, dummy);
    if (sem_val == -1)
        return -1;

    *isSet = (sem_val == 0) ? TRUE : FALSE;
    return 0;
}
