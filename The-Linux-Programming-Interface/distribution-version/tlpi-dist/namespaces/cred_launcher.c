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

/* cred_launcher.c

   Change the calling process's credentials to the numeric UID and GID
   provided by command-line options, and then launch the program (with
   arguments) specified in the remainder of the command line.

   A similar effect can be achieved using the setpriv(1) command:

        setpriv --reuid=1 --regid=1 --keep-group command [arg...]

   However, the program below also has an option (-v) to allow us to
   verify the process's capabilities before executing the command.
*/
#define _GNU_SOURCE
#include <unistd.h>
#include "userns_functions.h"
#include "tlpi_hdr.h"

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [-u UID] [-g GID] [-v] command [arg...]\n",
            pname);
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int newuid, newgid, verbose, opt;

    /* Parse command-line options */

    newuid = -1;
    newgid = -1;
    verbose = 0;
    while ((opt = getopt(argc, argv, "g:u:v")) != -1) {
        switch (opt) {
        case 'g': newgid = atoi(optarg);        break;
        case 'u': newuid = atoi(optarg);        break;
        case 'v': verbose = 1;                  break;
        default:  usage(argv[0]);
        }
    }

    if (argc <= optind)
        usage(argv[0]);

    /* Change process credentials as per the options */

    if (newgid != -1) {
        if (setresgid(newgid, newgid, newgid) == -1)
            errExit("setresuid");
    }

    if (newuid != -1) {
        if (setresuid(newuid, newuid, newuid) == -1)
            errExit("setresuid");
    }

    /* The '-v' option allows us to verify the capabilities of the process,
       which may have been modified as a consequence of UID changes */

    if (verbose)
        display_creds_and_caps("");

    /* Execute the command specified in the remaining arguments */

    execvp(argv[optind], &argv[optind]);
    errExit("execvp");

    exit(EXIT_SUCCESS);
}
