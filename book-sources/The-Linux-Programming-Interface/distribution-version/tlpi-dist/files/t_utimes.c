/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 15 */

/* t_utimes.c

   Demonstrate the use of utimes(): set the last modification time on a file to
   be the same as the last access time, but tweak the microsecond components of
   the two timestamps.

   Usage: t_utimes file

   See also t_utime.c.
*/
#include <sys/stat.h>
#include <sys/time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct stat sb;
    struct timeval tv[2];

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);

    if (stat(argv[1], &sb) == -1)       /* Retrieve current file times */
        errExit("stat");

    tv[0].tv_sec = sb.st_atime;         /* Leave atime seconds unchanged */
    tv[0].tv_usec = 223344;             /* Change microseconds for atime */
    tv[1].tv_sec = sb.st_atime;         /* mtime seconds == atime seconds */
    tv[1].tv_usec = 667788;             /* mtime microseconds */

    if (utimes(argv[1], tv) == -1)
        errExit("utimes");

    exit(EXIT_SUCCESS);
}
