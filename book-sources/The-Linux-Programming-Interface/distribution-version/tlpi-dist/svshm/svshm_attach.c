/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 48 */

/* svshm_attach.c

   Experiment with the use of shmat() to attach previously created
   System V shared memory segments.

   Usage: svshm_attach [shmid:addr[rR]]...

        r = attach with SHM_RND flag
        R = attach with SHM_RDONLY flag
*/
#include <sys/types.h>
#include <sys/shm.h>
#include "tlpi_hdr.h"

static void
usageError(char *progName)
{
    fprintf(stderr, "Usage: %s [shmid:address[rR]]...\n", progName);
    fprintf(stderr, "            r=SHM_RND; R=SHM_RDONLY\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    void *addr;
    char *retAddr, *p;
    int j, flags, shmid;

    printf("SHMLBA = %ld (%#lx), PID = %ld\n",
            (long) SHMLBA, (unsigned long) SHMLBA, (long) getpid());

    for (j = 1; j < argc; j++) {
        shmid = strtol(argv[j], &p, 0);
        if (*p != ':')
            usageError(argv[0]);

        addr = (void *) strtol(p + 1, NULL, 0);
        flags = (strchr(p + 1, 'r') != NULL) ? SHM_RND : 0;
        if (strchr(p + 1, 'R') != NULL)
            flags |= SHM_RDONLY;

        retAddr = shmat(shmid, addr, flags);
        if (retAddr == (void *) -1)
            errExit("shmat: %s", argv[j]);

        printf("%d: %s ==> %p\n", j, argv[j], retAddr);
    }

    printf("Sleeping 5 seconds\n");
    sleep(5);

    exit(EXIT_SUCCESS);
}
