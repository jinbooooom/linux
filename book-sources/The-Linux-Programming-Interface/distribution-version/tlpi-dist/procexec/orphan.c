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

   Demonstrate how a child becomes orphaned (and adopted by init(1))
   when its parent exits.

   Change history:
   2019-02-15   Changes to allow for the fact that on systems with a modern
                init(1) (e.g., systemd), an orphaned child may be adopted
                by a "child subreaper" process whose PID is not 1.
*/
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    pid_t ppid, ppidOrig;

    setbuf(stdout, NULL);       /* Disable buffering of stdout */

    ppidOrig = getpid();

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0:             /* Child */
        while ((ppid = getppid()) == ppidOrig) {   /* Loop until orphaned */
            printf("Child running (parent PID=%ld)\n", (long) ppid);
            sleep(1);
        }
        printf("Child is orphaned (parent PID=%ld)\n", (long) ppid);
        _exit(EXIT_SUCCESS);

    default:            /* Parent */
        printf("Parent (PID=%ld) sleeping\n", (long) getpid());
        sleep(3);                           /* Give child a chance to start */
        printf("Parent exiting\n");
        exit(EXIT_SUCCESS);
    }
}
