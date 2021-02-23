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

#include "scm_cred.h"

int
main(int argc, char *argv[])
{
    int data, lfd, sfd, optval, opt;
    ssize_t nr;
    bool useDatagramSocket;
    struct msghdr msgh;
    struct iovec iov;
    struct ucred rcred, scred;

    /* Allocate a char array of suitable size to hold the ancillary data.
       However, since this buffer is in reality a 'struct cmsghdr', use a
       union to ensure that it is aligned as required for that structure.
       Alternatively, we could allocate the buffer using malloc(), which
       returns a buffer that satisfies the strictest alignment
       requirements of any type */

    union {
        char   buf[CMSG_SPACE(sizeof(struct ucred))];
                        /* Space large enough to hold a 'ucred' structure */
        struct cmsghdr align;
    } controlMsg;
    struct cmsghdr *cmsgp;      /* Pointer used to iterate through
                                   headers in ancillary data */
    socklen_t len;

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

    /* We must set the SO_PASSCRED socket option in order to receive
       credentials */

    optval = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval)) == -1)
        errExit("setsockopt");

    /* The 'msg_name' field can be set to point to a buffer where the
       kernel will place the address of the peer socket. However, we don't
       need the address of the peer, so we set this field to NULL. */

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    /* Set fields of 'msgh' to point to buffer used to receive (real)
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
    printf("recvmsg() returned %zd\n", nr);

    if (nr > 0)
        printf("Received data = %d\n", data);

    /* Get the address of the first 'cmsghdr' in the received
       ancillary data */

    cmsgp = CMSG_FIRSTHDR(&msgh);

    /* Check the validity of the 'cmsghdr' */

    if (cmsgp == NULL || cmsgp->cmsg_len != CMSG_LEN(sizeof(struct ucred)))
        fatal("bad cmsg header / message length");
    if (cmsgp->cmsg_level != SOL_SOCKET)
        fatal("cmsg_level != SOL_SOCKET");
    if (cmsgp->cmsg_type != SCM_CREDENTIALS)
        fatal("cmsg_type != SCM_CREDENTIALS");

    /* Copy the contents of the data field of the 'cmsghdr' to a
       'struct ucred'. */

    memcpy(&rcred, CMSG_DATA(cmsgp), sizeof(struct ucred));

    /* Display the credentials from the received data area */

    printf("Received credentials pid=%ld, uid=%ld, gid=%ld\n",
                (long) rcred.pid, (long) rcred.uid, (long) rcred.gid);

    /* The Linux-specific, read-only SO_PEERCRED socket option returns
       credential information about the peer, as described in socket(7).
       This operation can be performed on UNIX domain stream sockets and on
       UNIX domain sockets (stream or datagram) created with socketpair(). */

    len = sizeof(struct ucred);
    if (getsockopt(sfd, SOL_SOCKET, SO_PEERCRED, &scred, &len) == -1)
        errExit("getsockopt");

    printf("Credentials from SO_PEERCRED: pid=%ld, euid=%ld, egid=%ld\n",
            (long) scred.pid, (long) scred.uid, (long) scred.gid);

    exit(EXIT_SUCCESS);
}
