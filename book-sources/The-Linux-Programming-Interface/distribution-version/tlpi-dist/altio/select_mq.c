/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 63-3 */

/* select_mq.c

   Usage: select_mq msqid...

   Demonstrate how we can use a child process in conjunction with
   select() in order to wait for input on a file descriptor (in this
   case the terminal) and on a message queue.

   This program allows us to monitor multiple message queues by
   creating a separate child for each queue named on its command line.

   For experimenting, you may find it useful to use the msg_create.c
   and msg_send.c programs from the System V IPC chapter.
*/
#include <sys/time.h>
#if ! defined(__hpux)
/* HP-UX 11 doesn't have this header file */
#include <sys/select.h>
#endif
#include <sys/msg.h>
#include <signal.h>
#include <stddef.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 200

/* Total size of the 'pbuf' struct must not exceed PIPE_BUF, otherwise
   writes by multiple children may not be atomic, with the result that
   messages are broken up and interleaved. */

#define MAX_MTEXT 400

struct pbuf {
    int msqid;                  /* Origin of message */
    int len;                    /* Number of bytes used in mtext */
    long mtype;                 /* Message type */
    char mtext[MAX_MTEXT];      /* Message body */
};

/* Function called by child: monitors message queue identified by
   'msqid', copying every message to the pipe identified by 'fd'. */

static void
childMon(int msqid, int fd)
{
    struct pbuf pmsg;
    ssize_t msgLen;
    size_t wlen;

    for (;;) {
        msgLen = msgrcv(msqid, &pmsg.mtype, MAX_MTEXT, 0, 0);
        if (msgLen == -1)
            errExit("msgrcv");

        /* We add some info to the message read by msgrcv() before
           writing to the pipe. */

        pmsg.msqid = msqid;
        pmsg.len = msgLen;      /* So parent knows how much to read from pipe */

        wlen = offsetof(struct pbuf, mtext) + msgLen;
        /* Or: wlen = &pmsg.mtext - &pmsg + msgLen */

        if (write(fd, &pmsg, wlen) != wlen)
            fatal("partial/failed write to pipe");
    }
}

int
main(int argc, char *argv[])
{
    fd_set readfds;
    int ready, nfds, j;
    int pfd[2];                 /* Pipe used to transfer messages from
                                   children to parent */
    ssize_t numRead;
    char buf[BUF_SIZE];
    struct pbuf pmsg;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s msqid...\n", argv[0]);

    if (pipe(pfd) == -1)
        errExit("pipe");

    /* Create one child for each message queue being monitored */

    for (j = 1; j < argc; j++) {
        switch (fork()) {
        case -1:
            errMsg("fork");
            killpg(0, SIGTERM);
            _exit(EXIT_FAILURE);        /* NOTREACHED */

        case 0:
            childMon(getInt(argv[j], 0, "msqid"), pfd[1]);
            _exit(EXIT_FAILURE);        /* NOTREACHED */

        default:
            break;
        }
    }

    /* Parent falls through to here */

    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(pfd[0], &readfds);
        nfds = max(STDIN_FILENO, pfd[0]) + 1;

        ready = select(nfds, &readfds, NULL, NULL, NULL);
        if (ready == -1)
            errExit("select");

        /* Check if terminal fd is ready */

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            numRead = read(STDIN_FILENO, buf, BUF_SIZE - 1);
            if (numRead == -1)
                errExit("read stdin");

            buf[numRead] = '\0';
            printf("Read from terminal: %s", buf);
            if (numRead > 0 && buf[numRead - 1] != '\n')
                printf("\n");
        }

        /* Check if pipe fd is ready */

        if (FD_ISSET(pfd[0], &readfds)) {
            numRead = read(pfd[0], &pmsg, offsetof(struct pbuf, mtext));
            if (numRead == -1)
                errExit("read pipe");
            if (numRead == 0)
                fatal("EOF on pipe");

            numRead = read(pfd[0], &pmsg.mtext, pmsg.len);
            if (numRead == -1)
                errExit("read pipe");
            if (numRead == 0)
                fatal("EOF on pipe");

            printf("MQ %d: type=%ld length=%d <%.*s>\n", pmsg.msqid,
                    pmsg.mtype, pmsg.len, pmsg.len, pmsg.mtext);
        }
    }
}
