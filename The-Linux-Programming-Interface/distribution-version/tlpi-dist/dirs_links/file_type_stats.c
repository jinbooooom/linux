/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 18-7 */

/* file_type_stats.c

   An example of the use of nftw(): traverse the directory tree named in the
   command line, and print out statistics about the types of file in the tree.
*/
#if defined(__sun)
#define _XOPEN_SOURCE 500   /* Solaris 8 needs it this way */
#else
#if ! defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 600
#define _XOPEN_SOURCE 600   /* Get nftw() and S_IFSOCK declarations */
#endif
#endif
#include <ftw.h>
#include "tlpi_hdr.h"

static int numReg = 0, numDir = 0, numSymLk = 0, numSocket = 0,
           numFifo = 0, numChar = 0, numBlock = 0,
           numNonstatable = 0;

static int
countFile(const char *path, const struct stat *sb, int flag, struct FTW *ftwb)

{
    if (flag == FTW_NS) {
        numNonstatable++;
        return 0;
    }

    switch (sb->st_mode & S_IFMT) {
    case S_IFREG:  numReg++;    break;
    case S_IFDIR:  numDir++;    break;
    case S_IFCHR:  numChar++;   break;
    case S_IFBLK:  numBlock++;  break;
    case S_IFLNK:  numSymLk++;  break;
    case S_IFIFO:  numFifo++;   break;
    case S_IFSOCK: numSocket++; break;
    }
    return 0;           /* Always tell nftw() to continue */
}

static void
printStats(const char *msg, int num, int numFiles)
{
    printf("%-15s   %6d %6.1f%%\n", msg, num, num * 100.0 / numFiles);
}

int
main(int argc, char *argv[])
{
    int numFiles;       /* Total number of files */

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s dir-path\n", argv[0]);

    /* Traverse directory tree counting files; don't follow symbolic links */

    if (nftw(argv[1], &countFile, 20, FTW_PHYS) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }

    numFiles = numReg + numDir + numSymLk + numSocket +
                numFifo + numChar + numBlock + numNonstatable;

    if (numFiles == 0) {
        printf("No files found\n");
    } else {
        printf("Total files:      %6d\n", numFiles);
        printStats("Regular:", numReg, numFiles);
        printStats("Directory:", numDir, numFiles);
        printStats("Char device:", numChar, numFiles);
        printStats("Block device:", numBlock, numFiles);
        printStats("Symbolic link:", numSymLk, numFiles);
        printStats("FIFO:", numFifo, numFiles);
        printStats("Socket:", numSocket, numFiles);
        printStats("Non-statable:", numNonstatable, numFiles);
    }
    exit(EXIT_SUCCESS);
}
