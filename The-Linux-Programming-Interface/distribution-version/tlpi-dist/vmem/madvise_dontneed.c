/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 50-2 */

/* madvise_dontneed.c

   Demonstrate the "destructive" semantics of the madvise() MADV_DONTNEED
   operation. On Linux, when MADV_DONTNEED is applied to a MAP_PRIVATE mapping,
   the pages (and thus any modifications to the pages) are discarded; when
   next accessed, the pages are reinitialized from the underlying file.

   NOTE: MADV_DONTNEED is not destructive on some UNIX implementations.

   The mincore() system call is supported on Linux since kernel 2.4.
*/
#ifdef __linux__
#define _BSD_SOURCE
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define MAP_SIZE 4096
#define WRITE_SIZE 10

int
main(int argc, char *argv[])
{
    char *addr;
    int fd, j;

    setbuf(stdout, NULL);

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);

    unlink(argv[1]);
    fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    for (j = 0; j < MAP_SIZE; j++)
        write(fd, "a", 1);
    if (fsync(fd) == -1)
        errExit("fsync");
    close(fd);

    fd = open(argv[1], O_RDWR);
    if (fd == -1)
        errExit("open");

    addr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    printf("After mmap:          ");
    write(STDOUT_FILENO, addr, WRITE_SIZE);
    printf("\n");

    /* Copy-on-write semantics mean that the following modification
       will create private copies of the pages for this process */

    for (j = 0; j < MAP_SIZE; j++)
        addr[j]++;

    printf("After modification:  ");
    write(STDOUT_FILENO, addr, WRITE_SIZE);
    printf("\n");

    /* After the following, the mapping contents revert to the original file
       contents (if MADV_DONTNEED has destructive semantics, as on Linux) */

    if (madvise(addr, MAP_SIZE, MADV_DONTNEED) == -1)
        errExit("madvise");

    printf("After MADV_DONTNEED: ");
    write(STDOUT_FILENO, addr, WRITE_SIZE);
    printf("\n");

    exit(EXIT_SUCCESS);
}
