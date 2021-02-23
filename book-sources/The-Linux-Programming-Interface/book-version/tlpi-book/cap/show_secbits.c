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

#include <sys/prctl.h>
#include "cap_functions.h"
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int secbits = prctl(PR_GET_SECUREBITS, 0, 0, 0, 0);
    if (secbits == -1)
        errExit("prctl");

    printf("secbits = 0x%x => ", secbits);

    printSecbits(secbits, argc == 1, stdout);
    printf("\n");

    exit(EXIT_SUCCESS);
}
