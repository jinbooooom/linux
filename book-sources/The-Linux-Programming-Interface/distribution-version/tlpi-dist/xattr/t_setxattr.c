/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 16 */

/* t_setxattr.c

   Demonstrate the use of setxattr() to set a file extended attribute.

   This program is Linux (2.6 and later) specific.

   See also view_xattr.c.
*/
#include <sys/xattr.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    char *value;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);

    value = "The past is not dead.";
    if (setxattr(argv[1], "user.x", value, strlen(value), 0) == -1)
        errExit("setxattr");

    value = "In fact, it's not even past.";
    if (setxattr(argv[1], "user.y", value, strlen(value), 0) == -1)
        errExit("setxattr");

    exit(EXIT_SUCCESS);
}
