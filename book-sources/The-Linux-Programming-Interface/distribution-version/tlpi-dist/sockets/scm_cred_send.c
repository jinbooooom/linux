/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 61 */

/* scm_cred_send.c

   Used in conjunction with scm_cred_recv.c to demonstrate passing of
   process credentials via a UNIX domain socket.

   This program sends credentials to a UNIX domain socket.

   Usage is as shown in the usageErr() call below.

   Credentials can be exchanged over stream or datagram sockets. This program
   uses stream sockets by default; the "-d" command-line option specifies
   that datagram sockets should be used instead.

   This program is Linux-specific.

   See also scm_multi_send.c.
*/
#include "scm_cred.h"

int
main(int argc, char *argv[])
{
    int data, sfd, opt;
    ssize_t ns;
    bool useDatagramSocket, noExplicitCreds;
    struct msghdr msgh;
    struct iovec iov;

    /* Allocate a char array of suitable size to hold the ancillary data.
       However, since this buffer is in reality a 'struct cmsghdr', use a
       union to ensure that it is aligned as required for that structure.
       Alternatively, we could allocate the buffer using malloc(), which
       returns a buffer that satisfies the strictest alignment
       requirements of any type */

    union {
        char   buf[CMSG_SPACE(sizeof(struct ucred))];
                        /* Space large enough to hold a ucred structure */
        struct cmsghdr align;
    } controlMsg;
    struct cmsghdr *cmsgp;      /* Pointer used to iterate through
                                   headers in ancillary data */

    /* Parse command-line options */

    useDatagramSocket = false;
    noExplicitCreds = false;

    while ((opt = getopt(argc, argv, "dn")) != -1) {
        switch (opt) {
        case 'd':
            useDatagramSocket = true;
            break;

        case 'n':
            noExplicitCreds = true;
            break;

        default:
            usageErr("%s [-d] [-n] [data [PID [UID [GID]]]]\n"
                    "        -d    use datagram socket\n"
                    "        -n    don't construct explicit "
                                  "credentials structure\n", argv[0]);
        }
    }

    /* The 'msg_name' field can be used to specify the address of the
       destination socket when sending a datagram. However, we do not
       need to use this field because we use connect() below, which sets
       a default outgoing address for datagrams. */

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    /* On Linux, we must transmit at least 1 byte of real data in
       order to send ancillary data */

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);

    /* Data is optionally taken from command line */

    data = (argc > optind) ? atoi(argv[optind]) : 12345;
    fprintf(stderr, "Sending data = %d\n", data);

    if (noExplicitCreds) {

        /* Don't construct an explicit credentials structure. (It is not
           necessary to do so, if we just want the receiver to receive
           our real credentials.) */

        printf("Not explicitly sending a credentials structure\n");
        msgh.msg_control = NULL;
        msgh.msg_controllen = 0;

    } else {

        /* Set 'msgh' fields to describe the ancillary data buffer */

        msgh.msg_control = controlMsg.buf;
        msgh.msg_controllen = sizeof(controlMsg.buf);

        /* The control message buffer must be zero-initialized in order for the
           CMSG_NXTHDR() macro to work correctly. Although we don't need to use
           CMSG_NXTHDR() in this example (because there is only one block of
           ancillary data), we show this step to demonstrate best practice */

        memset(controlMsg.buf, 0, sizeof(controlMsg.buf));

        /* Set message header to describe the ancillary data that
           we want to send */

        cmsgp = CMSG_FIRSTHDR(&msgh);
        cmsgp->cmsg_len = CMSG_LEN(sizeof(struct ucred));
        cmsgp->cmsg_level = SOL_SOCKET;
        cmsgp->cmsg_type = SCM_CREDENTIALS;

        /* Use sender's own PID, real UID, and real GID, unless
           alternate values were supplied on the command line */

        struct ucred creds;

        creds.pid = getpid();
        if (argc > optind + 1 && strcmp(argv[optind + 1], "-") != 0)
            creds.pid = atoi(argv[optind + 1]);

        creds.uid = getuid();
        if (argc > optind + 2 && strcmp(argv[optind + 2], "-") != 0)
            creds.uid = atoi(argv[optind + 2]);

        creds.gid = getgid();
        if (argc > optind + 3 && strcmp(argv[optind + 3], "-") != 0)
            creds.gid = atoi(argv[optind + 3]);

        printf("Send credentials pid=%ld, uid=%ld, gid=%ld\n",
                (long) creds.pid, (long) creds.uid, (long) creds.gid);

        /* Copy 'ucred' structure into data field in the 'cmsghdr' */

        memcpy(CMSG_DATA(cmsgp), &creds, sizeof(struct ucred));
    }

    /* Connect to the peer socket */

    sfd = unixConnect(SOCK_PATH, useDatagramSocket ? SOCK_DGRAM : SOCK_STREAM);
    if (sfd == -1)
        errExit("unixConnect");

    /* Send real plus ancillary data */

    ns = sendmsg(sfd, &msgh, 0);
    if (ns == -1)
        errExit("sendmsg");

    printf("sendmsg() returned %zd\n", ns);

    exit(EXIT_SUCCESS);
}
