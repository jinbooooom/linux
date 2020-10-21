/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter Z */

/* seccomp_launch.c

   Usage: seccomp_launch [-f bpf-filter-blob]... prog arg...

   Launch a program with arguments, after first (optionally) loading
   previously generated BPF filter(s) from specified file(s).
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static int
seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(__NR_seccomp, operation, flags, args);
}

static void
loadFilter(char *filterPathname)
{
    static bool noNewPrivsAlreadySet = false;
    struct sock_fprog fprog;
    struct sock_filter *filter;
    int filterSize;
    struct stat sb;
    int fd;

    if (!noNewPrivsAlreadySet) {        /* Only need to do this once */
        if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
            errExit("prctl");
        noNewPrivsAlreadySet = true;
    }

    /* Open the file, determine its size, allocate a buffer of that size,
       and read the file into the buffer */

    fd = open(filterPathname, O_RDONLY);
    if (fd == -1)
        errExit("open");

    if (fstat(fd, &sb) == -1)
        errExit("fstat");

    filterSize = sb.st_size;
    if (filterSize % sizeof(struct sock_filter) != 0) {
        fprintf(stderr, "Filter has odd size\n");
        exit(EXIT_FAILURE);
    }

    filter = malloc(filterSize);
    if (filter == NULL)
        errExit("malloc");

    if (read(fd, filter, filterSize) != filterSize) {
        fprintf(stderr, "Failure reading filter\n");
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1)
        errExit("close");

    /* Install the BPF filter blob */

    fprog.len = filterSize / sizeof(struct sock_filter);
    fprog.filter = filter;

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &fprog) == -1)
        errExit("seccomp");
}

static void
usageError(char *pname, char *msg)
{
    fprintf(stderr, "%s", msg);
    fprintf(stderr, "Usage: %s [-f filter] prog arg...\n", pname);
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int opt;

    /* Command-line parsing */

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {

        case 'f':               /* Install a filter */
            loadFilter(optarg);
            break;

        default:
            usageError(argv[0], "Bad option\n");
        }
    }

    if (optind >= argc || strcmp(argv[1], "--help") == 0)
        usageError(argv[0], "No program specified\n");

    /* Execute program named on command line */

    execvp(argv[optind], &argv[optind]);
    errExit("execve");
}
