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

/* t_cap_set_file.c

   Usage: ./t_cap_set_file "<textual-cap-set>" <pathname>
*/
#include <sys/capability.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    cap_t capSets;
    char *textCaps;

    if (argc != 3) {
        fprintf(stderr, "%s <textual-cap-set> <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    capSets = cap_from_text(argv[1]);
    if (capSets == NULL)
        errExit("cap_from_text");

    textCaps = cap_to_text(capSets, NULL);
    if (textCaps == NULL)
        errExit("cap_to_text");

    printf("caps_to_text() returned \"%s\"\n\n", textCaps);

    if (cap_set_file(argv[2], capSets) == -1)
        errExit("cap_set_file");

    if (cap_free(textCaps) != 0 || cap_free(capSets) != 0)
        errExit("cap_free");

    exit(EXIT_SUCCESS);
}
