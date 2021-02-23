/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 5-6 */

#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd1, fd2, fd3;
#define file "a"
    char cmd[] = "cat " file "; echo";

    fd1 = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd1 == -1)
        errExit("open fd1");
    fd2 = dup(fd1);
    if (fd2 == -1)
        errExit("dup");
    fd3 = open(file, O_RDWR);
    if (fd3 == -1)
        errExit("open fd3");

    /* 'fd1' and 'fd2' share same open file table entry (and thus file
       offset). 'fd3' has its own open file table entry, and thus a
       separate file offset. */

    if (write(fd1, "Hello,", 6) == -1)
        errExit("write1");
    system(cmd);
    if (write(fd2, " world", 6) == -1)
        errExit("write2");
    system(cmd);
    if (lseek(fd2, 0, SEEK_SET) == -1)
        errExit("lseek");
    if (write(fd1, "HELLO,", 6) == -1)
        errExit("write3");
    system(cmd);
    if (write(fd3, "Gidday", 6) == -1)
        errExit("write4");
    system(cmd);

    if (close(fd1) == -1)
        errExit("close output");
    if (close(fd2) == -1)
        errExit("close output");
    if (close(fd3) == -1)
        errExit("close output");
    exit(EXIT_SUCCESS);
}
