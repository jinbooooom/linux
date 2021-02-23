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
