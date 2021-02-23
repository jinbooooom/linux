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

#include "scm_rights.h"

int
main(int argc, char *argv[])
{
    int data, sfd, opt, fd;
    ssize_t ns;
    bool useDatagramSocket;
    struct msghdr msgh;
    struct iovec iov;

    /* Allocate a char array of suitable size to hold the ancillary data.
       However, since this buffer is in reality a 'struct cmsghdr', use a
       union to ensure that it is aligned as required for that structure.
       Alternatively, we could allocate the buffer using malloc(), which
       returns a buffer that satisfies the strictest alignment
       requirements of any type. */

    union {
        char   buf[CMSG_SPACE(sizeof(int))];
                        /* Space large enough to hold an 'int' */
        struct cmsghdr align;
    } controlMsg;
    struct cmsghdr *cmsgp;      /* Pointer used to iterate through
                                   headers in ancillary data */

    /* Parse command-line options */

    useDatagramSocket = false;

    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
        case 'd':
            useDatagramSocket = true;
            break;

        default:
            usageErr("%s [-d] file\n"
                     "        -d    use datagram socket\n", argv[0]);
        }
    }

    if (argc != optind + 1)
        usageErr("%s [-d] file\n", argv[0]);

    /* Open the file named on the command line */

    fd = open(argv[optind], O_RDONLY);
    if (fd == -1)
        errExit("open");

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
    data = 12345;
    fprintf(stderr, "Sending data = %d\n", data);

    /* Set 'msgh' fields to describe the ancillary data buffer */

    msgh.msg_control = controlMsg.buf;
    msgh.msg_controllen = sizeof(controlMsg.buf);

    /* The control message buffer must be zero-initialized in order
       for the CMSG_NXTHDR() macro to work correctly. Although we
       don't need to use CMSG_NXTHDR() in this example (because
       there is only one block of ancillary data), we show this
       step to demonstrate best practice */

    memset(controlMsg.buf, 0, sizeof(controlMsg.buf));

    /* Set message header to describe the ancillary data that
       we want to send */

    cmsgp = CMSG_FIRSTHDR(&msgh);
    cmsgp->cmsg_len = CMSG_LEN(sizeof(int));
    cmsgp->cmsg_level = SOL_SOCKET;
    cmsgp->cmsg_type = SCM_RIGHTS;
    memcpy(CMSG_DATA(cmsgp), &fd, sizeof(int));

    /* Connect to the peer socket */

    sfd = unixConnect(SOCK_PATH, useDatagramSocket ? SOCK_DGRAM : SOCK_STREAM);
    if (sfd == -1)
        errExit("unixConnect");

    fprintf(stderr, "Sending FD %d\n", fd);

    /* Send real plus ancillary data */

    ns = sendmsg(sfd, &msgh, 0);
    if (ns == -1)
        errExit("sendmsg");

    fprintf(stderr, "sendmsg() returned %zd\n", ns);

    /* Once the file descriptor has been sent, it is no longer necessary
       to keep it open in the sending process */

    if (close(fd) == -1)
        errExit("close");

    exit(EXIT_SUCCESS);
}
