/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 47-5 */

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include "semun.h"                      /* Definition of semun union */
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int semid, key, perms;
    struct sembuf sops[2];

    key = 12345;
    perms = S_IRUSR | S_IWUSR;
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | perms);

    if (semid != -1) {                  /* Successfully created the semaphore */
        union semun arg;

        /* XXXX */

        arg.val = 0;                    /* So initialize it */
        if (semctl(semid, 0, SETVAL, arg) == -1)
            errExit("semctl");

    } else {                            /* We didn't create semaphore set */
        if (errno != EEXIST) {          /* Unexpected error from semget() */
            errExit("semget 1");
        } else {                        /* Someone else already created it */
            semid = semget(key, 1, perms);      /* So just get ID */
            if (semid == -1)
                errExit("semget 2");
        }
    }

    /* Now perform some operation on the semaphore */

    sops[0].sem_op = 1;         /* Add 1 */
    sops[0].sem_num = 0;        /* ... to semaphore 0 */
    sops[0].sem_flg = 0;
    if (semop(semid, sops, 1) == -1)
        errExit("semop");

    exit(EXIT_SUCCESS);
}
