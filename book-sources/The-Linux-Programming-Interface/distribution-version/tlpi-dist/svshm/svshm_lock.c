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

/* svshm_lock.c

   Lock the System V shared memory segments identified by the
   command-line arguments.

   See also svshm_unlock.c.
*/
#include <sys/types.h>
#include <sys/shm.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int j;

    for (j = 1; j < argc; j++)
        if (shmctl(getInt(argv[j], 0, "shmid"), SHM_LOCK, NULL) == -1)
            errExit("shmctl");

    sleep(5);

    exit(EXIT_SUCCESS);
}
