/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 15-7 */

/* chiflag.c

   Change i-node flags (sometimes also known as ext2 extended file attributes)
   for files named on the command line. Usage is as shown in usageError() below.

   This program won't build on a system that takes its <linux/fs.h> from
   a kernel earlier than 2.6.19. You'll instead need to do something
   like including <linux/ext2_fs.h> and replacing all of the FS_* names
   below by EXT2_*.

   This program is Linux-specific.
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include "tlpi_hdr.h"

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s {+-=}{attrib-chars} file...\n\n", progName);
#define fpe(str) fprintf(stderr, "    " str)            /* Shorter! */
    fpe("+ add attribute; - remove attribute; "
                        "= set attributes absolutely\n\n");
    fpe("'attrib-chars' contains one or more of:\n");
    fpe("    a   Force open() to include O_APPEND "
                        "(privilege required)\n");
    fpe("    A   Do not update last access time\n");
    fpe("    c   Compress (requires e2compr package)\n");
    fpe("    d   Do not backup with dump(8)\n");
    fpe("    D   Synchronous directory updates\n");
    fpe("    i   Immutable (privilege required)\n");
    fpe("    j   Enable ext3/ext4 data journaling\n");
    fpe("    s   Secure deletion (not implemented)\n");
    fpe("    S   Synchronous file updates\n");
    fpe("    t   Disable tail-packing (Reiserfs only)\n");
    fpe("    T   Mark as top-level directory for Orlov algorithm\n");
    fpe("    u   Undelete (not implemented)\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int attr, oldAttr, j, fd;
    char *p;

    if (argc < 3 || strchr("+-=", argv[1][0]) == NULL ||
            strcmp(argv[1], "--help") == 0)
        usageError(argv[0]);

    /* Build bit mask based on attribute string in argv[1] */

    attr = 0;
    for (p = &argv[1][1]; *p != '\0'; p++) {
        switch (*p) {
        case 'a': attr |= FS_APPEND_FL;         break;
        case 'A': attr |= FS_NOATIME_FL;        break;
        case 'c': attr |= FS_COMPR_FL;          break;
        case 'd': attr |= FS_NODUMP_FL;         break;
        case 'D': attr |= FS_DIRSYNC_FL;        break;
        case 'i': attr |= FS_IMMUTABLE_FL;      break;
        case 'j': attr |= FS_JOURNAL_DATA_FL;   break;
        case 's': attr |= FS_SECRM_FL;          break;
        case 'S': attr |= FS_SYNC_FL;           break;
        case 't': attr |= FS_NOTAIL_FL;         break;
        case 'T': attr |= FS_TOPDIR_FL;         break;
        case 'u': attr |= FS_UNRM_FL;           break;
        default:  usageError(argv[0]);
        }
    }

    /* Open each filename in turn, and change its attributes */

    for (j = 2; j < argc; j++) {
        fd = open(argv[j], O_RDONLY);
        if (fd == -1) {         /* Most likely error is nonexistent file */
            errMsg("open: %s", argv[j]);
            continue;
        }

        /* If argv[1] starts with + or -, then retrieve existing
           attribute bit mask and modify as required */

        if (argv[1][0] == '+' || argv[1][0] == '-') {
            if (ioctl(fd, FS_IOC_GETFLAGS, &oldAttr) == -1)
                errExit("ioctl1: %s", argv[j]);
            attr = (*argv[1] == '-') ? (oldAttr & ~attr) : (oldAttr | attr);
        }

        if (ioctl(fd, FS_IOC_SETFLAGS, &attr) == -1)
            errExit("ioctl2: %s", argv[j]);
        if (close(fd) == -1)
            errExit("close");
    }

    exit(EXIT_SUCCESS);
}
