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

/* epoll_flags_fork.c

   A program to experiment with various epoll flags, in particular EPOLLET
   and EPOLLEXCLUSIVE.

   Usage: ./epoll_flags_fork [options] <FIFO> <num-children>

   Example usage, in this case to experiment with EPOLLEXCLUSIVE where five
   child processes each create their own epoll FD to which they all add the
   file descriptor for the read end of a FIFO:

        mkfifo p
        ./epoll_flags_fork -x p 5

   and then in another window, run the following command and type lines of
   input:

        cat > p

   To explore the difference when not using EPOLLEXCLUSIVE, repeat the
   above but run the program without the '-x' option:

        ./epoll_flags_fork p 5

   To explore the behavior where EPOLLET wakes up only one of multiple
   waiters on the same epoll FD, use the '-e' and '-s' options:

        ./epoll_flags_fork -es p 5
*/
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

#ifndef EPOLLEXCLUSIVE
#define EPOLLEXCLUSIVE (1 << 28)
#endif

static void
usageError(char *pname)
{
    fprintf(stderr, "Usage: %s [-1eoprx] <FIFO> <num-children>\n",
            pname);
    fprintf(stderr, "\t-s       Create one epoll FD before creating child "
            "processes\n");
    fprintf(stderr, "\t\t(By default, each child creates its own epoll FD "
            "after fork())\n");
    fprintf(stderr, "\t-e       Include EPOLLET flag\n");
    fprintf(stderr, "\t-x       Include EPOLLEXCLUSIVE flag\n");
    fprintf(stderr, "\t-o       Include EPOLLONESHOT flag\n");
    fprintf(stderr, "\t-p       Open FIFO individually in each child\n");
    fprintf(stderr, "\t\t(By default, each child inherits FD for FIFO opened "
            "by parent)\n");
    fprintf(stderr, "\t-r       Do a read() after epoll_wait() returns\n");
    fprintf(stderr, "\t-l       Children should loop, rather than "
            "calling epoll_wait() just once\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int fd, epfd, numReady;
    struct epoll_event ev, rev;
    int childNum, childMax;
    int eventsMask, opt;
    int useOneEpollFD, readData, openFifoInChild, useLoop;
    char *fifoPath;

    /* Parse command-line options and arguments */

    useOneEpollFD = 0;
    readData = 0;
    eventsMask = EPOLLIN;
    openFifoInChild = 0;
    useLoop = 0;
    while ((opt = getopt(argc, argv, "eloprsx")) != -1) {
        switch (opt) {
        case 'e': eventsMask |= EPOLLET;        break;
        case 'o': eventsMask |= EPOLLONESHOT;   break;
        case 'x': eventsMask |= EPOLLEXCLUSIVE; break;
        case 'l': useLoop = 1;                  break;
        case 'p': openFifoInChild = 1;          break;
        case 'r': readData = 1;                 break;
        case 's': useOneEpollFD = 1;            break;
        default:  usageError(argv[0]);
        }
    }

    if (argc != optind + 2 || strcmp(argv[optind], "--help") == 0)
        usageError(argv[0]);

    fifoPath = argv[optind];
    childMax = atoi(argv[optind + 1]);

    /* Either we open the FIFO once in the parent (and each child inherits
       the file descriptor from the parent, or each child opens the FIFO
       after fork() */

    if (!openFifoInChild) {
        fd = open(fifoPath, O_RDONLY | O_NONBLOCK);
        if (fd == -1)
            errExit("open");
        printf("Opened FIFO %s\n", fifoPath);
    }

    /* Either we create the epoll FD once in the parent (and it is inherited by
       each child) and add the FIFO to the interest list of the epoll instance,
       or we perform these steps in each of the children after fork() */

    if (useOneEpollFD) {
        printf("Creating single epoll FD and adding FIFO\n");
        epfd = epoll_create(2);
        if (epfd == -1)
            errExit("epoll_create");

        ev.events = eventsMask;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
            errExit("epoll_ctl");
    }

    printf("\n");

    /* Create child processes */

    for (childNum = 0; childNum < childMax; childNum++) {
        switch (fork()) {
        case -1:
            errExit("fork");

        case 0: /* Child */
            printf("Child %d: created\n", childNum);
            if (openFifoInChild) {
                fd = open(fifoPath, O_RDONLY | O_NONBLOCK);
                if (fd == -1)
                    errExit("open");
                printf("Child %d: opened FIFO %s\n", childNum, fifoPath);
            }

            if (!useOneEpollFD) {
                printf("Child %d: creating epoll FD and adding FIFO\n",
                        childNum);
                epfd = epoll_create(2);
                if (epfd == -1)
                    errExit("epoll_create");

                ev.events = eventsMask;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
                    errExit("epoll_ctl");
            }

            do {
                /* Wait on the epoll FD and print results */

                printf("Child %d: about to epoll_wait()\n", childNum);
                numReady = epoll_wait(epfd, &rev, 1, -1);
                if (numReady == -1)
                    errExit("epoll-wait");
                printf("Child %d: epoll_wait() returned %d\n", childNum,
                        numReady);

                /* If specified on command line, read data when the FIFO
                   becomes ready */

                if (readData) {
                    ssize_t nr;
                    char buf[50000];

                    usleep(50000);
                    nr = read(fd, buf, sizeof(buf));
                    if (nr == 0) {
                        printf("Child %d: read returned EOF\n", childNum);
                        break;
                    } else if (nr > 0) {
                        printf("Child %d: read returned %zd bytes\n",
                                childNum, nr);
                    } else {
                        printf("Child %d: read failed: %s\n", childNum,
                                strerror(errno));
                    }
                }
            } while (useLoop);

            printf("Child %d: terminating\n", childNum);
            exit(EXIT_SUCCESS);

        default:
            break;
        }
    }

    usleep(50000);
    printf("======================\n");

    for (childNum = 0; childNum < childMax; childNum++)
        wait(NULL);

    exit(EXIT_SUCCESS);
}
