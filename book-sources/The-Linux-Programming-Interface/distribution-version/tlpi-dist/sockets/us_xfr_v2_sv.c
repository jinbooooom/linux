/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 59-3:d */

/* us_xfr_v2_sv.c

   An example UNIX stream socket server. Accepts incoming connections and
   copies data sent from clients to stdout. This program is similar to
   us_xfr_sv.c, except that it uses the functions in unix_sockets.c to
   simplify working with UNIX domain sockets.

   See also us_xfr_v2_cl.c.
*/
#include "us_xfr_v2.h"

int
main(int argc, char *argv[])
{
    int sfd, cfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = unixBind(SV_SOCK_PATH, SOCK_STREAM);
    if (sfd == -1)
        errExit("unixBind");

    if (listen(sfd, 5) == -1)
        errExit("listen");

    for (;;) {          /* Handle client connections iteratively */
        cfd = accept(sfd, NULL, NULL);
        if (cfd == -1)
            errExit("accept");

        /* Transfer data from connected socket to stdout until EOF */

        while ((numRead = read(cfd, buf, BUF_SIZE)) > 0)
            if (write(STDOUT_FILENO, buf, numRead) != numRead)
                fatal("partial/failed write");

        if (numRead == -1)
            errExit("read");
        if (close(cfd) == -1)
            errMsg("close");
    }
}
