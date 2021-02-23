/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 44-1 */

/* change_case.c

   Demonstrate the use of two pipes for bidirectional communication
   between a parent and child process. The parent reads text from
   standard input and sends it to the child via one of the pipes.
   The child reads text from this pipe, converts it to uppercase, and
   sends it back to the parent using the other pipe. The parent reads
   the text returned by the child and echoes it on standard output.
*/
#include <ctype.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 100    /* Should be <= PIPE_BUF bytes */

int
main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    int outbound[2];            /* Pipe to send data from parent to child */
    int inbound[2];             /* Pipe to send data from child to parent */
    int j;
    ssize_t cnt;

    if (pipe(outbound) == -1)
        errExit("pipe");
    if (pipe(inbound) == -1)
        errExit("pipe");

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child */

        /* Close unused pipe descriptors */

        if (close(outbound[1]) == -1)
            errExit("close");
        if (close(inbound[0]) == -1)
            errExit("close");

        /* Read data from outbound pipe, convert to uppercase,
           and send back to parent on inbound pipe */

        while ((cnt = read(outbound[0], buf, BUF_SIZE)) > 0) {
            for (j = 0; j < cnt; j++)
                buf[j] = toupper((unsigned char) buf[j]);
            if (write(inbound[1], buf, cnt) != cnt)
                fatal("failed/partial write(): inbound pipe");
        }

        if (cnt == -1)
            errExit("read");
        _exit(EXIT_SUCCESS);

    default:

        /* Close unused pipe descriptors */

        if (close(outbound[0]) == -1)
            errExit("close");
        if (close(inbound[1]) == -1)
            errExit("close");

        /* Read data from stdin, send to the child via the
           outbound pipe, read the results back from the child
           on the inbound pipe, and print them on stdout */

        while ((cnt = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
            if (write(outbound[1], buf, cnt) != cnt)
                fatal("failed/partial write(): outbound pipe");

            cnt = read(inbound[0], buf, BUF_SIZE);
            if (cnt == -1)
                errExit("read");
            if (cnt > 0)
                if (write(STDOUT_FILENO, buf, cnt) != cnt)
                    fatal("failed/partial write(): STDOUT_FILENO");
        }

        if (cnt == -1)
            errExit("read");

        /* Exiting will close write end of outbound pipe, so that
           child see EOF */

        exit(EXIT_SUCCESS);
    }
}
