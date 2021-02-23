/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 39 */

/* t_cap_get_file.c

   Display the capabilities attached to a file.
*/
#include <sys/capability.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

int
main(int argc, char *argv[])
{
    cap_t caps;
    char *str;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Fetch and display file capabilities */

    caps = cap_get_file(argv[1]);

    if (caps == NULL) {
        if (errno == ENODATA)
            printf("No capabilities are attached to this file\n");
        else
            errExit("cap_get_file");
    } else {
        str = cap_to_text(caps, NULL);
        if (str == NULL)
            errExit("cap_to_text");

        printf("Capabilities: %s\n", str);

        cap_free(str);
    }

    cap_free(caps);

    exit(EXIT_SUCCESS);
}
