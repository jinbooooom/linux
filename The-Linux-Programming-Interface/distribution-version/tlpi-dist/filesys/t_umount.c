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

/* t_umount.c

   Demonstrate the use of the umount() system call to unmount a mount point.

   Usage: t_umount mount-point

   This program is Linux-specific.
*/
#include <sys/mount.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s mount-point\n", argv[0]);

    if (umount(argv[1]) == -1)
        errExit("umount");

    exit(EXIT_SUCCESS);
}
