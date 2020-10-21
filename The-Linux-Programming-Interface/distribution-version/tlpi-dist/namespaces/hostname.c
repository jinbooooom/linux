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

/* hostname.c

   Display or change the system hostname.

   Usage: hostname [new-host-name]
*/
#define _BSD_SOURCE
#include <sys/param.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE (MAXHOSTNAMELEN + 1)

int
main(int argc, char *argv[])
{
    char buf[BUF_SIZE];

    if (argc > 1) {
        if (sethostname(argv[1], strlen(argv[1])) == -1) {
            perror("sethostname");
            exit(EXIT_FAILURE);
        }
    } else {
        if (gethostname(buf, BUF_SIZE) == -1) {
            perror("gethostname");
            exit(EXIT_FAILURE);
        }
        printf("%s\n", buf);
    }

    exit(EXIT_SUCCESS);
}
