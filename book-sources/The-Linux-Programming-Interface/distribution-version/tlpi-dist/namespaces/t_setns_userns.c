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

/* t_setns.c

   Attempt to join a user namespace using setns(), displaying
   process's credentials and capabilities before and after setns().
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/capability.h>
#include "userns_functions.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

int
main(int argc, char *argv[])
{
    int fd;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s /proc/PID/ns/FILE\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    display_creds_and_caps("Initial:\n");
    printf("\n");

    fd = open(argv[1], O_RDONLY); /* Get descriptor for namespace */
    if (fd == -1)
        errExit("open");

    if (setns(fd, CLONE_NEWUSER) == -1)  /* Join that namespace */
        errExit("setns-1");

    display_creds_and_caps("After setns():\n");
    exit(EXIT_SUCCESS);
}
