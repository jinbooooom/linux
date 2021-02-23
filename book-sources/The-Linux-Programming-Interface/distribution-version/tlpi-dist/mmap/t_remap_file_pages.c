/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 49 */

/* t_remap_file_pages.c

   Demonstrate the use of the Linux remap_file_pages() system call to create
   a nonlinear mapping.

   This program is Linux-specific. The remap_file_pages() system call is
   supported since kernel 2.6.
*/
#define _GNU_SOURCE             /* Get remap_file_pages() declaration
                                   from <sys/mman.h> */
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd, j;
    char ch;
    long pageSize;
    char *addr;

    fd = open("/tmp/tfile", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize == -1)
        fatal("Couldn't determine page size");

    for (ch = 'a'; ch < 'd'; ch++)
        for (j = 0; j < pageSize; j++)
            write(fd, &ch, 1);

    system("od -a /tmp/tfile");

    addr = mmap(NULL, 3 * pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    printf("Mapped at address %p\n", addr);

    /* The three pages of the file -- 0 1 2 -- are currently mapped
       linearly. Now we rearrange the mapping to 2 1 0. */

    if (remap_file_pages(addr, pageSize, 0, 2, 0) == -1)
        errExit("remap_file_pages");
    if (remap_file_pages(addr + 2 * pageSize, pageSize, 0, 0, 0) == -1)
        errExit("remap_file_pages");

    /* Now we modify the contents of the mapping */

    for (j = 0; j < 0x100; j++)         /* Modifies page 2 of file */
        *(addr + j) = '0';
    for (j = 0; j < 0x100; j++)         /* Modifies page 0 of file */
        *(addr + 2 * pageSize + j) = '2';

    system("od -a /tmp/tfile");

    exit(EXIT_SUCCESS);
}
