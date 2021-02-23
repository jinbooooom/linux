/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 6 */

/* t_getenv.c

   Demonstrate the use of getenv() to retrieve the value of an
   environment variable.
*/
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    char *val;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s environ-var\n", argv[0]);

    val = getenv(argv[1]);
    printf("%s\n", (val != NULL) ? val : "No such variable");

    exit(EXIT_SUCCESS);
}
