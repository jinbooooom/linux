/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 62-2 */

/* ttyname.c

   An implementation of ttyname(3).
*/
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Helper function for ttyname(). We do most of the real work here.
   Look in 'devDir' for the terminal device name corresponding to the
   stat structure given in 'fdStat' (which the caller must ensure
   derives from a terminal device).

   Return the device name (as a statically-allocated) string if
   found, or NULL if the device is not found or an error occurs */

static char *
ttynameCheckDir(const struct stat *fdStat, const char *devDir)
{
    DIR *dirh;
    struct dirent *dent;
    static char *ttyPath;               /* Currently checked entry; also used
                                           to return tty name, if found */
    static int ttyLen = 0;              /* Length of ttyPath */
    struct stat devStat;                /* stat entry for ttyPath */
    int found;                          /* True if we find device entry */
    int requiredLen;

    if (ttyLen == 0) {                  /* First call - allocate ttyPath */
        ttyPath = malloc(50);
        if (ttyPath == NULL)
            return NULL;
        ttyLen = 50;
    }

    dirh = opendir(devDir);
    if (dirh == NULL)
        return NULL;

    /* We walk through each file in /dev looking for an entry whose
       device ID (st_rdev) matches the device ID corresponding to 'fd'.
       To do this, we construct a pathname for each entry in /dev, and
       then call stat() on that pathname. This is somewhat expensive.
       The glibc implementation of ttyname() performs an optimization:
       it performs a first pass of the entries in /dev, performing
       a stat() call only if fdStat->st_ino == dent->d_ino. This speeds
       the search (since many calls to stat() are avoided), but is not
       guaranteed to work in every case (e.g., if there are symbolic
       links in /dev). Therefore, if the first pass fails to find a
       matching device, glibc's ttyname() performs a second pass
       without the st_ino check (i.e., like we do below). */

    found = 0;
    while ((dent = readdir(dirh)) != NULL) {
        requiredLen = strlen(devDir) + 1 + strlen(dent->d_name) + 1;

        if (requiredLen > ttyLen) {     /* Resize ttyPath if required */
            char *nttyPath;

            nttyPath = realloc(ttyPath, requiredLen);
            if (nttyPath == NULL)
                break;

            ttyPath = nttyPath;
            ttyLen = requiredLen;
        }

        snprintf(ttyPath, ttyLen, "%s/%s", devDir, dent->d_name);

        if (stat(ttyPath, &devStat) == -1)
            continue;                   /* Ignore unstat-able entries */

        if (S_ISCHR(devStat.st_mode) &&
                fdStat->st_rdev == devStat.st_rdev) {
            found = 1;
            break;
        }
    }

    closedir(dirh);

    return found ? ttyPath : NULL;
}

/* Return the name of the terminal associated with 'fd' (in a
   statically-allocated string that is overwritten on subsequent
   calls), or NULL if it is not a terminal or an error occurs */

char *
ttyname(int fd)
{
    char *d;
    struct stat fdStat;                 /* stat entry for fd */

    if (!isatty(fd))                    /* Is fd even a terminal? */
        return NULL;

    if (fstat(fd, &fdStat) == -1)
        return NULL;

    if (!S_ISCHR(fdStat.st_mode))       /* Is fd even a character device? */
        return NULL;

    /* First check for a pseudoterminal entry in the /dev/pts
       directory. If that fails, try looking in /dev.
       (We check the directories in this order for efficiency:
       /dev/pts is small, and will contain the required device if
       fd refers to an X-terminal or similar.)  */

    d = ttynameCheckDir(&fdStat, "/dev/pts");
    return (d != NULL) ? d : ttynameCheckDir(&fdStat, "/dev");
}
