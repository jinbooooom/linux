/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 63 */

/* multithread_epoll_wait.c

   If multiple threads are waiting in epoll_wait() on the same FD, then,
   with EPOLLET (edge-triggered notification), only one of the threads
   is woken up when I/O activity occurs. By contrast, with level-triggered
   notification, all threads are woken up.

   To test this:

        $ ./multithread_epoll_wait x            # With EPOLLET
        Thread 0 about to epoll_wait()
        Thread 1 about to epoll_wait()
        Thread 2 about to epoll_wait()
        Thread 3 about to epoll_wait()
        Thread 4 about to epoll_wait()

        main() about to write a byte to pipe

        Thread 4 completed epoll_wait(); ready = 1
        main() about to terminate

        $ ./multithread_epoll_wait              # Without EPOLLET
        Thread 1 about to epoll_wait()
        Thread 4 about to epoll_wait()
        Thread 0 about to epoll_wait()
        Thread 3 about to epoll_wait()
        Thread 2 about to epoll_wait()

        main() about to write a byte to pipe

        Thread 2 completed epoll_wait(); ready = 1
        Thread 3 completed epoll_wait(); ready = 1
        Thread 0 completed epoll_wait(); ready = 1
        Thread 4 completed epoll_wait(); ready = 1
        Thread 1 completed epoll_wait(); ready = 1
        main() about to terminate
*/
#include <sys/epoll.h>
#include <fcntl.h>
#include <pthread.h>
#include "tlpi_hdr.h"

#define MAX_EVENTS     5      /* Max. # of events we allow to be returned
                                 from a single epoll_wait() call */

static int pipe1[2];
static int epfd;

static void *
threadFunc(void *arg)
{
    struct epoll_event evlist[MAX_EVENTS];
    long tnum = (long) arg;
    int ready;

    printf("Thread %ld about to epoll_wait()\n", tnum);
    ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
    if (ready == -1)
        errExit("epoll_wait");
    printf("Thread %ld completed epoll_wait(); ready = %d\n", tnum, ready);

    return NULL;
}

int
main(int argc, char *argv[])
{
    int s;
    struct epoll_event ev;
    pthread_t t1;
    int epollet;

    epollet = (argc > 1) ? EPOLLET : 0;

    epfd = epoll_create(5);
    if (epfd == -1)
        errExit("epoll_create");

    if (pipe(pipe1) == -1)
        errExit("pipe1");

    ev.events = EPOLLIN | epollet;      /* Only interested in input events */
    ev.data.fd = pipe1[0];
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipe1[0], &ev) == -1)
        errExit("epoll_ctl");

    for (long j = 0; j < 5; j++) {
        s = pthread_create(&t1, NULL, threadFunc, (void *) j);
        if (s != 0)
            errExitEN(s, "pthread_create");
    }

    sleep(2);

    printf("\nmain() about to write a byte to pipe\n\n");
    write(pipe1[1], "x", 1);
    sleep(2);
    printf("main() about to terminate\n");

    exit(EXIT_SUCCESS);

}
