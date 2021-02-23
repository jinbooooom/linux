/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 5 */

/* t_truncate.c

   Demonstrate the use of the truncate() system call to truncate the file
   named in argv[1] to the length specified in argv[2]
*/
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file length\n", argv[0]);

    if (truncate(argv[1], getLong(argv[2], GN_ANY_BASE, "length")) == -1)
        errExit("truncate");

    exit(EXIT_SUCCESS);
}
