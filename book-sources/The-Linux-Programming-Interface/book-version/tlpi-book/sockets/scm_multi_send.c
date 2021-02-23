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

#define _GNU_SOURCE
#include "scm_multi.h"

static void
usageError(char *pname)
{
    fprintf(stderr, "Usage: %s [options] file...\n", pname);
    fprintf(stderr, "    Options:\n");
    fprintf(stderr, "\t-d        Use datagram (instead of stream) socket\n");
    fprintf(stderr, "\t-n        Don't send any real data with the "
                    "ancillary data\n");
    fprintf(stderr, "\t-p <pid>  Use this PID when sending credentials\n");
    fprintf(stderr, "\t-u <uid>  Use this UID when sending credentials\n");
    fprintf(stderr, "\t-g <gid>  Use this GID when sending credentials\n");
    fprintf(stderr, "    If any of any of -p/-u/-g is absent, the "
                    "corresponding real\n    credential is used.\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int data, sfd, opt;
    pid_t pid;
    uid_t uid;
    gid_t gid;
    ssize_t numSent;
    bool useDatagramSocket, sendData;
    struct msghdr msgh;
    struct iovec iov;
    int fdCnt;                  /* Number of FDs in ancillary data */
    char *controlMsg;           /* Ancillary data (control message) */
    size_t controlMsgSize;      /* Size of ancillary data */
    struct cmsghdr *cmsgp;      /* Pointer used to iterate through
                                   headers in ancillary data */

    /* By default, this program sends an SCM_CREDENTIALS message containing
       the process's real credentials. This can be altered via command-line
       options. */

    pid = getpid();
    uid = getuid();
    gid = getgid();

    /* Parse command-line options */

    useDatagramSocket = false;
    sendData = true;

    while ((opt = getopt(argc, argv, "dnp:u:g:")) != -1) {
        switch (opt) {
        case 'd':
            useDatagramSocket = true;
            break;

        case 'n':
            sendData = false;
            break;

        case 'p':
            pid = atoi(optarg);
            break;

        case 'u':
            uid = atoi(optarg);
            break;

        case 'g':
            gid = atoi(optarg);
            break;

        default:
            usageError(argv[0]);
        }
    }

    fdCnt = argc - optind;
    if (fdCnt <= 0)
        usageError(argv[0]);

    /* Allocate a buffer of suitable size to hold the ancillary data.
       This buffer is in reality treated as a 'struct cmsghdr',
       and so needs to be suitably aligned: malloc() provides a block
       with suitable alignment. */

    controlMsgSize = CMSG_SPACE(sizeof(int) * fdCnt) +
                     CMSG_SPACE(sizeof(struct ucred));
    controlMsg = malloc(controlMsgSize);
    if (controlMsg == NULL)
        errExit("malloc");

    /* The control message buffer must be zero-initialized in order for
       the CMSG_NXTHDR() macro to work correctly */

    memset(controlMsg, 0, controlMsgSize);

    /* The 'msg_name' field can be used to specify the address of the
       destination socket when sending a datagram. However, we do not
       need to use this field because we use connect() below, which sets
       a default outgoing address for datagrams. */

    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    /* On Linux, we must transmit at least 1 byte of real data in
       order to send ancillary data, at least when using stream sockets.
       The following allows for testing the results if no real data is
       sent with the ancillary data. */

    if (sendData) {
        msgh.msg_iov = &iov;
        msgh.msg_iovlen = 1;
        iov.iov_base = &data;
        iov.iov_len = sizeof(int);
        data = 12345;
    } else {
        msgh.msg_iov = NULL;
        msgh.msg_iovlen = 0;
    }

    /* Place a pointer to the ancillary data, and size of that data,
       in the 'msghdr' structure that will be passed to sendmsg() */

    msgh.msg_control = controlMsg;
    msgh.msg_controllen = controlMsgSize;

    /* Set message header to describe the ancillary data that
       we want to send */

    /* First, the file descriptor list */

    cmsgp = CMSG_FIRSTHDR(&msgh);
    cmsgp->cmsg_level = SOL_SOCKET;
    cmsgp->cmsg_type = SCM_RIGHTS;

    /* The ancillary message must include space for the required number
       of file descriptors */

    cmsgp->cmsg_len = CMSG_LEN(sizeof(int) * fdCnt);
    printf("cmsg_len 1: %ld\n", (long) cmsgp->cmsg_len);

    /* Open files named on the command line, and copy the resulting block of
       file descriptors into the data field of the ancillary message */

    int *fdList = calloc(fdCnt, sizeof(int));
    if (fdList == NULL)
        errExit("calloc");

    /* Open the files named on the command line, placing the returned file
       descriptors into the ancillary data */

    for (int j = 0; j < fdCnt; j++) {
         fdList[j] = open(argv[optind + j], O_RDONLY);
         if (fdList[j] == -1)
             errExit("open");
    }

    memcpy(CMSG_DATA(cmsgp), fdList, fdCnt * sizeof(int));

    /* Next, the credentials */

    cmsgp = CMSG_NXTHDR(&msgh, cmsgp);
    cmsgp->cmsg_level = SOL_SOCKET;
    cmsgp->cmsg_type = SCM_CREDENTIALS;

    /* The ancillary message must include space for a 'struct ucred' */

    cmsgp->cmsg_len = CMSG_LEN(sizeof(struct ucred));
    printf("cmsg_len 2: %ld\n", (long) cmsgp->cmsg_len);

    /* Initialize the credentials that are to be placed in the ancillary data */

    struct ucred creds;

    creds.pid = pid;
    creds.uid = uid;
    creds.gid = gid;

    /* Copy credentilas to the data area of this ancillary message block */

    memcpy(CMSG_DATA(cmsgp), &creds, sizeof(struct ucred));

    /* Connect to the peer socket */

    sfd = unixConnect(SOCK_PATH, useDatagramSocket ? SOCK_DGRAM : SOCK_STREAM);
    if (sfd == -1)
        errExit("unixConnect");

    /* Send the data plus ancillary data */

    numSent = sendmsg(sfd, &msgh, 0);
    if (numSent == -1)
        errExit("sendmsg");

    printf("sendmsg() returned %zd\n", numSent);

    exit(EXIT_SUCCESS);
}
