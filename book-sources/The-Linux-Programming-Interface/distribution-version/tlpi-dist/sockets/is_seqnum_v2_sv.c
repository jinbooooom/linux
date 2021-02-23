/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 59-2:b */

/* is_seqnum_v2_sv.c

   A simple Internet stream socket server. Our service is to provide unique
   sequence numbers to the client.

   This program is the same as is_seqnum_cl.c, except that it uses the functions
   in our inet_sockets.c library to simplify set up of the server's socket.

   Usage:  is_seqnum_sv [init-seq-num]  (default = 0)

   See also is_seqnum_v2_cl.c.
*/
#include "is_seqnum_v2.h"

int
main(int argc, char *argv[])
{
    uint32_t seqNum;
    char reqLenStr[INT_LEN];            /* Length of requested sequence */
    char seqNumStr[INT_LEN];            /* Start of granted sequence */
    struct sockaddr *claddr;
    int lfd, cfd, reqLen;
    socklen_t addrlen, alen;
    char addrStr[IS_ADDR_STR_LEN];

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [init-seq-num]\n", argv[0]);

    seqNum = (argc > 1) ? getInt(argv[1], 0, "init-seq-num") : 0;

    /* Ignore the SIGPIPE signal, so that we find out about broken connection
       errors via a failure from write(). */

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    errExit("signal");

    lfd = inetListen(PORT_NUM_STR, 5, &addrlen);
    if (lfd == -1)
        fatal("inetListen() failed");

    /* Allocate a buffer large enough to hold the client's socket address */

    claddr = malloc(addrlen);
    if (claddr == NULL)
        errExit("malloc");

    for (;;) {                  /* Handle clients iteratively */

        /* Accept a client connection, obtaining client's address */

        alen = addrlen;
        cfd = accept(lfd, (struct sockaddr *) claddr, &alen);
        if (cfd == -1) {
            errMsg("accept");
            continue;
        }

        printf("Connection from %s\n", inetAddressStr(claddr, alen,
                        addrStr, IS_ADDR_STR_LEN));

        /* Read client request, send sequence number back */

        if (readLine(cfd, reqLenStr, INT_LEN) <= 0) {
            close(cfd);
            continue;                   /* Failed read; skip request */
        }

        reqLen = atoi(reqLenStr);
        if (reqLen <= 0) {              /* Watch for misbehaving clients */
            close(cfd);
            continue;                   /* Bad request; skip it */
        }

        snprintf(seqNumStr, INT_LEN, "%d\n", seqNum);
        if (write(cfd, seqNumStr, strlen(seqNumStr)) != strlen(seqNumStr))
            fprintf(stderr, "Error on write");

        seqNum += reqLen;               /* Update sequence number */

        if (close(cfd) == -1)           /* Close connection */
            errMsg("close");
    }
}
