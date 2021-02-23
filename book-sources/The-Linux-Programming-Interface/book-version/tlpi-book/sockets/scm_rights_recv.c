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

#define BUF_SIZE 100

int
main(int argc, char *argv[])
{
    int data, lfd, sfd, fd, opt;
    ssize_t nr;
    bool useDatagramSocket;
    struct msghdr msgh;
    struct iovec iov;

    /* Allocate a char array of suitable size to hold the ancillary data.
       However, since this buffer is in reality a 'struct cmsghdr', use a
       union to ensure that it is aligned as required for that structure.
       Alternatively, we could allocate the buffer using malloc(), which
       returns a buffer that satisfies the strictest alignment
       requirements of any type */

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
            usageErr("%s [-d]\n"
                     "        -d    use datagram socket\n", argv[0]);
        }
    }

    /* Create socket bound to a well-known address. In the case where
       we are using stream sockets, also make the socket a listening
       socket and accept a connection on the socket. */

    if (remove(SOCK_PATH) == -1 && errno != ENOENT)
        errExit("remove-%s", SOCK_PATH);

    if (useDatagramSocket) {
        sfd = unixBind(SOCK_PATH, SOCK_DGRAM);
        if (sfd == -1)
            errExit("unixBind");

    } else {
        lfd = unixBind(SOCK_PATH, SOCK_STREAM);
        if (lfd == -1)
            errExit("unixBind");

        if (listen(lfd, 5) == -1)
            errExit("listen");

        sfd = accept(lfd, NULL, NULL);
        if (sfd == -1)
            errExit("accept");
    }

    /* The 'msg_name' field can be set to point to a buffer where the
       kernel will place the address of the peer socket. However, we don't
       need the address of the peer, so we set this field to NULL. */

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    /* Set fields of 'msgh' to point to buffer used to receive the (real)
       data read by recvmsg() */

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);

    /* Set 'msgh' fields to describe the ancillary data buffer */

    msgh.msg_control = controlMsg.buf;
    msgh.msg_controllen = sizeof(controlMsg.buf);

    /* Receive real plus ancillary data */

    nr = recvmsg(sfd, &msgh, 0);
    if (nr == -1)
        errExit("recvmsg");
    fprintf(stderr, "recvmsg() returned %zd\n", nr);

    if (nr > 0)
        fprintf(stderr, "Received data = %d\n", data);

    /* Get the address of the first 'cmsghdr' in the received
       ancillary data */

    cmsgp = CMSG_FIRSTHDR(&msgh);

    /* Check the validity of the 'cmsghdr' */

    if (cmsgp == NULL || cmsgp->cmsg_len != CMSG_LEN(sizeof(int)))
        fatal("bad cmsg header / message length");
    if (cmsgp->cmsg_level != SOL_SOCKET)
        fatal("cmsg_level != SOL_SOCKET");
    if (cmsgp->cmsg_type != SCM_RIGHTS)
        fatal("cmsg_type != SCM_RIGHTS");

    /* The data area of the 'cmsghdr' is an 'int' (a file descriptor);
       copy that integer to a local variable. (The received file descriptor
       is typically a different file descriptor number than was used in the
       sending process.) */

    memcpy(&fd, CMSG_DATA(cmsgp), sizeof(int));
    fprintf(stderr, "Received FD %d\n", fd);

    /* Having obtained the file descriptor, read the file's contents and
       print them on standard output */

    for (;;) {
        char buf[BUF_SIZE];
        ssize_t numRead;

        numRead = read(fd, buf, BUF_SIZE);
        if (numRead == -1)
            errExit("read");

        if (numRead == 0)
            break;

        write(STDOUT_FILENO, buf, numRead);
    }

    exit(EXIT_SUCCESS);
}
