/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 5-3 */

/* atomic_append.c

   Demonstrate the difference between using nonatomic lseek()+write()
   and O_APPEND when writing to a file.

   Usage: file num-bytes [x]

   The program write 'num-bytes' bytes to 'file' a byte at a time. If
   no additional command-line argument is supplied, the program opens the
   file with the O_APPEND flag. If a command-line argument is supplied, the
   O_APPEND is omitted when calling open(), and the program calls lseek()
   to seek to the end of the file before calling write(). This latter
   technique is vulnerable to a race condition, where data is lost because
   the lseek() + write() steps are not atomic. This can be demonstrated
   by looking at the size of the files produced by these two commands:

        atomic_append f1 1000000 & atomic_append f1 1000000

        atomic_append f2 1000000 x & atomic_append f2 1000000 x
*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int numBytes, j, flags, fd;
    Boolean useLseek;

    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file num-bytes [x]\n"
                 "        'x' means use lseek() instead of O_APPEND\n",
                 argv[0]);

    useLseek = argc > 3;
    flags = useLseek ? 0 : O_APPEND;
    numBytes = getInt(argv[2], 0, "num-bytes");

    fd = open(argv[1], O_RDWR | O_CREAT | flags, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    for (j = 0; j < numBytes; j++) {
        if (useLseek)
            if (lseek(fd, 0, SEEK_END) == -1)
                errExit("lseek");
        if (write(fd, "x", 1) != 1)
            fatal("write() failed");
    }

    printf("%ld done\n", (long) getpid());
    exit(EXIT_SUCCESS);
}
