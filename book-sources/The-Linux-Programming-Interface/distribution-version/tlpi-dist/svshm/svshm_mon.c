/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 48-4 */

/* svshm_mon.c

   Display information from the associated data structure for the
   System V shared memory segment identified on the command line.
*/
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include "tlpi_hdr.h"

static void
printShmDS(const struct shmid_ds *ds)
{
    printf("Size:                      %ld\n", (long) ds->shm_segsz);
    printf("# of attached processes:   %ld\n", (long) ds->shm_nattch);

    printf("Mode:                      %lo",
            (unsigned long) ds->shm_perm.mode);
#ifdef SHM_DEST
    printf("%s", (ds->shm_perm.mode & SHM_DEST) ? " [DEST]" : "");
#endif
#ifdef SHM_LOCKED
    printf("%s", (ds->shm_perm.mode & SHM_LOCKED) ? " [LOCKED]" : "");
#endif
    printf("\n");

    printf("Last shmat():              %s", ctime(&ds->shm_atime));
    printf("Last shmdt():              %s", ctime(&ds->shm_dtime));
    printf("Last change:               %s", ctime(&ds->shm_ctime));

    printf("Creator PID:               %ld\n", (long) ds->shm_cpid);
    printf("PID of last attach/detach: %ld\n", (long) ds->shm_lpid);
}

int
main(int argc, char *argv[])
{
    struct shmid_ds ds;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s shmid\n", argv[0]);

    if (shmctl(getInt(argv[1], 0, "shmid"), IPC_STAT, &ds) == -1)
        errExit("shmctl");

    printShmDS(&ds);

    exit(EXIT_SUCCESS);
}
