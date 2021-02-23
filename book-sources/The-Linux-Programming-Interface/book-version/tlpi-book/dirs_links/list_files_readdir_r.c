/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 18-4 */

#include <limits.h>
#include <dirent.h>
#include <stddef.h>
#include "tlpi_hdr.h"

static void             /* List all files in directory 'dirpath' */
listFiles(const char *dirpath)
{
    DIR *dirp;
    Boolean isCurrent;          /* True if 'dirpath' is "." */
    struct dirent *result, *entryp;
    int nameMax;

    isCurrent = strcmp(dirpath, ".") == 0;

    /* On Linux, NAME_MAX is defined in <limits.h>. However, this limit
       may vary across file systems, so we really should use pathconf()
       to find the true limit for this file system. */

    nameMax = pathconf(dirpath, _PC_NAME_MAX);
    if (nameMax == -1)          /* Indeterminate or error */
        nameMax = 255;          /* So take a guess */

    entryp = malloc(offsetof(struct dirent, d_name) + nameMax + 1);
    if (entryp == NULL)
        errExit("malloc");

    /* Open the directory - on failure print an error and return */

    dirp = opendir(dirpath);
    if (dirp == NULL) {
        errMsg("opendir failed on '%s'", dirpath);
        free(entryp);
        return;
    }

    /* Look at each of the entries in this directory */

    for (;;) {
        errno = readdir_r(dirp, entryp, &result);
        if (errno != 0)
            errExit("readdir_r");

        if (result == NULL)     /* End of stream */
            break;

        /* Skip . and .. */

        if (strcmp(entryp->d_name, ".") == 0 ||
                strcmp(entryp->d_name, "..") == 0)
            continue;

        /* Print directory + filename */

        if (!isCurrent) printf("%s/", dirpath);
        printf("%s\n", entryp->d_name);
    }

    if (closedir(dirp) == -1)
        errMsg("closedir");
}

int
main(int argc, char *argv[])
{
    if (argc == 1)              /* No arguments - use current directory */
        listFiles(".");
    else
        for (argv++; *argv; argv++)
            listFiles(*argv);
    exit(EXIT_SUCCESS);
}
