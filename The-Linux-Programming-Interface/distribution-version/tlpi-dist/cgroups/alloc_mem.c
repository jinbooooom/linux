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

/* alloc_mem.c

  Allocate blocks of memory, as specified by command-line arguments. This
  program is useful for small experiments with the 'memory' cgroup controller.
*/
#define _GNU_SOURCE
#include <pthread.h>
#include <sys/syscall.h>
#include "tlpi_hdr.h"

static size_t blockSize;
static int sleepUsecs, numAllocs, numThreads;

static void
allocMem(int numAllocs, size_t blockSize, int sleepUsecs)
{
    size_t totalMem = 0;
    char *p;

    if (numThreads > 1) {

        /* If we have created multiple threads, then pause until the user hits
           <enter>.  This gives the user a chance to split the threads across
           multiple (v1) memory cgroups that have different memory limits,
           making it possible to see how the kernel enforces the limit(s).
           (Some light experimentation suggests that only the limit on the
           thread group leader is enforced.) */

        char ch;
        printf("Hit ENTER when ready to start memory allocation "
                "(PID = %ld; TID = %ld)\n", (long) getpid(),
                syscall(SYS_gettid));
        read(STDIN_FILENO, &ch, 1);
    }

    for (int j = 0; (numAllocs == -1) || (j < numAllocs); j++) {
        p = malloc(blockSize);
        if (p == NULL)
            errExit("malloc %d", j);

        /* Make sure virtual memory is actually allocated by touching
           every page */

        for (int k = 0; k < blockSize; k += 1024)
            p[k] = 0;

        totalMem += blockSize;
        printf("%4d: address = %p; total = 0x%zx (%zd MiB)\n",
                j, p, totalMem, totalMem / (1024 * 1024));

        /* If the user requested, slow things down by sleeping
           for some microseconds between each allocation */

        if (sleepUsecs != 0)
            usleep(sleepUsecs);
    }

    /* Pause, so that we hold onto allocated memory */

    printf("All memory allocated; pausing\n");
    pause();
}

static void *
threadFunc(void *arg)
{
    int doAlloc = (long) arg;

    if (doAlloc)
        allocMem(numAllocs, blockSize, sleepUsecs);

    pause();
    return NULL;
}

static void
usageError(char *pname)
{
    fprintf(stderr, "Usage: %s block-size num-allocs [sleep-usecs "
            "[tflag...]]\n\n", pname);
    fprintf(stderr, "Allocate 'num-allocs' blocks of memory of size "
            "'block-size' bytes.\n\n");
    fprintf(stderr, "The optional 'sleep-usecs' (default: 0) specifies a "
            "number of microseconds to\n");
    fprintf(stderr, "sleep between each allocation.\n\n");
    fprintf(stderr, "One additional thread is created for each 'tflag' "
            "argument\n\n");
    fprintf(stderr, "'tflag' is either '+' or '.'. At most one 'tflag' can be "
            "'+', and memory\n"
            "         allocation is done in that thread, or otherwise in the "
            "main thread\n"
            "         if no 'tflag' was '+'.\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    bool allocated = false;

    if (argc < 3)
        usageError(argv[0]);

    blockSize = strtol(argv[1], NULL, 0);
    numAllocs = atoi(argv[2]);
    sleepUsecs = (argc > 3) ? atoi(argv[3]) : 0;

    numThreads = argc - 4;

    if (argc > 4) {

        /* If the user requested the creation of additional threads, then
           create those threads. */

        for (int j = 4; j < argc; j++) {
            pthread_t thr;
            long doAlloc;

            doAlloc = argv[j][0] == '+';
            if (doAlloc && allocated)
                fatal("Can only specify one '+' argument");

            int s = pthread_create(&thr, NULL, threadFunc, (void *) doAlloc);
            if (s != 0)
                errExitEN(s, "pthread_create");

            if (doAlloc)
                allocated = true;
        }

        printf("All threads created\n");
    }

    if ( ! allocated)
        allocMem(numAllocs, blockSize, sleepUsecs);

    pause();
    exit(EXIT_SUCCESS);
}
