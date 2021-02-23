/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 59-3:e */

/* us_xfr_v2_cl.c

   An example UNIX domain stream socket client. This client transmits contents
   of stdin to a server socket. This program is similar to us_xfr_cl.c, except
   that it uses the functions in unix_sockets.c to simplify working with UNIX
   domain sockets.

   See also us_xfr_v2_sv.c.
*/
#include "us_xfr_v2.h"

int
main(int argc, char *argv[])
{
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = unixConnect(SV_SOCK_PATH, SOCK_STREAM);
    if (sfd == -1)
        errExit("unixConnect");

    /* Copy stdin to socket */

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
        if (write(sfd, buf, numRead) != numRead)
            fatal("partial/failed write");

    if (numRead == -1)
        errExit("read");
    exit(EXIT_SUCCESS);     /* Closes our socket; server sees EOF */
}
