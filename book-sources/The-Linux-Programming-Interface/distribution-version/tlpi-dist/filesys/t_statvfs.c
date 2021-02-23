/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 14 */

/* t_statvfs.c

   Demonstrate the use of statvfs() to retrieve information about
   a mounted file system.

   See also t_statfs.c.
*/
#include <sys/statvfs.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct statvfs sb;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s path\n", argv[0]);

    if (statvfs(argv[1], &sb) == -1)
        errExit("statvfs");

    printf("Block size                       %lu\n", sb.f_bsize);
    printf("Fundamental block size           %lu\n", sb.f_frsize);
    printf("Total blocks (in above units)    %lu\n",
            (unsigned long) sb.f_blocks);
    printf("Free blocks for priv. proc.      %lu\n",
            (unsigned long) sb.f_bfree);
    printf("Free blocks for unpriv. proc.    %lu\n",
            (unsigned long) sb.f_bavail);
    printf("Total number of i-nodes          %lu\n",
            (unsigned long) sb.f_files);
    printf("Free i-nodes for priv. proc.     %lu\n",
            (unsigned long) sb.f_ffree);
    printf("Free i-nodes for nonpriv. proc.  %lu\n",
            (unsigned long) sb.f_favail);
    printf("File system ID                   %#lx\n", sb.f_fsid);
    printf("Flags                            %#lx\n", sb.f_flag);
    printf("Maximum filename length          %lu\n", sb.f_namemax);

    exit(EXIT_SUCCESS);
}
