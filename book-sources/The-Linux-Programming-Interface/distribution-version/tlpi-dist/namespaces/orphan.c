/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter Z */

/* orphan.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Demonstrate that a child becomes orphaned (and is adopted by init(1),
   whose PID is 1) when its parent exits.

   See https://lwn.net/Articles/532748/

   Change history:
   2019-02-15   Changes to allow for the fact that on systems with a modern
                init(1) (e.g., systemd), an orphaned child may be adopted
                by a "child subreaper" process whose PID is not 1.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
    pid_t pid, ppidOrig;

    ppidOrig = getpid();

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid != 0) {             /* Parent */
        printf("Parent (PID=%ld) created child with PID %ld\n",
                (long) getpid(), (long) pid);
        printf("Parent (PID=%ld; PPID=%ld) terminating\n",
                (long) getpid(), (long) getppid());
        exit(EXIT_SUCCESS);
    }

    /* Child falls through to here */

    do {
        usleep(100000);
    } while (getppid() == ppidOrig);            /* Am I an orphan yet? */

    printf("\nChild  (PID=%ld) now an orphan (parent PID=%ld)\n",
            (long) getpid(), (long) getppid());

    sleep(1);

    printf("Child  (PID=%ld) terminating\n", (long) getpid());
    _exit(EXIT_SUCCESS);
}
