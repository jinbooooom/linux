/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 36-2:a */

/* rusage.c

   Execute a command and then print a summary of the resources (as retrieved
   by getrusage()) that it used.

   See also print_rudage.c.
*/
#include <sys/resource.h>
#include <sys/wait.h>
#include "print_rusage.h"
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    pid_t childPid;
    struct rusage ru;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s command arg...\n", argv[0]);

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0:
        execvp(argv[1], &argv[1]);
        errExit("execvp");

    default:
        printf("Command PID: %ld\n", (long) childPid);
        if (wait(NULL) == -1)
            errExit("wait");
        if (getrusage(RUSAGE_CHILDREN, &ru) == -1)
            errExit("getrusage");

        printf("\n");
        printRusage("\t", &ru);
        exit(EXIT_SUCCESS);
    }
}
