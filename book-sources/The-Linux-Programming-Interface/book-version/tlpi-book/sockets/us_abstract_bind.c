/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 57-8 */

#include <sys/un.h>
#include <sys/socket.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_un addr;
    char *str;

    memset(&addr, 0, sizeof(struct sockaddr_un));  /* Clear address structure */
    addr.sun_family = AF_UNIX;                     /* UNIX domain address */

    /* addr.sun_path[0] has already been set to 0 by memset() */

    str = "xyz";        /* Abstract name is "\0xyz" */
    strncpy(&addr.sun_path[1], str, strlen(str));

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
        errExit("socket");

    if (bind(sockfd, (struct sockaddr *) &addr,
            sizeof(sa_family_t) + strlen(str) + 1) == -1)
        errExit("bind");

    sleep(60);

    exit(EXIT_SUCCESS);
}
