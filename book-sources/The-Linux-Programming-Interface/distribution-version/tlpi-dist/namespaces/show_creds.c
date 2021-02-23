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

/* show_creds.c

   Display process UIDs and GIDs.
*/
#define _GNU_SOURCE
#include <sys/capability.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

int
main(int argc, char *argv[])
{
    cap_t caps;
    char *s;

    for (;;) {
        caps = cap_get_proc();
        if (caps == NULL)
            errExit("cap_get_proc");

        printf("eUID = %ld;  eGID = %ld;  ",
                (long) geteuid(), (long) getegid());

        s = cap_to_text(caps, NULL);
        if (s == NULL)
            errExit("cap_to_text");
        printf("capabilities: %s\n", s);

        cap_free(caps);
        cap_free(s);

        if (argc == 1)
            break;

        sleep(5);
    }
    exit(EXIT_SUCCESS);
}
