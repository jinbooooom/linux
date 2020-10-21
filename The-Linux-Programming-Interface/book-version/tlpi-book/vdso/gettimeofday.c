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

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <sys/syscall.h>
#include <sys/time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct timeval curr;
    long lim;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s loop-count\n", argv[0]);

    lim = atoi(argv[1]);

    for (int j = 0; j < lim; j++) {
#ifdef USE_SYSCALL
        if (syscall(__NR_gettimeofday, &curr, NULL) == -1)
            errExit("gettimeofday");
#else
        if (gettimeofday(&curr, NULL) == -1)
            errExit("gettimeofday");
#endif
    }

    exit(EXIT_SUCCESS);
}
