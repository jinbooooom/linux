/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 61-3 */

/* sendfile.c

   Implement sendfile() in terms of read(), write(), and lseek().
*/
#include "tlpi_hdr.h"
#define BUF_SIZE 8192

ssize_t
sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    off_t orig;
    char buf[BUF_SIZE];
    size_t toRead, numRead, numSent, totSent;

    if (offset != NULL) {

        /* Save current file offset and set offset to value in '*offset' */

        orig = lseek(in_fd, 0, SEEK_CUR);
        if (orig == -1)
            return -1;
        if (lseek(in_fd, *offset, SEEK_SET) == -1)
            return -1;
    }

    totSent = 0;

    while (count > 0) {
        toRead = min(BUF_SIZE, count);

        numRead = read(in_fd, buf, toRead);
        if (numRead == -1)
            return -1;
        if (numRead == 0)
            break;                      /* EOF */

        numSent = write(out_fd, buf, numRead);
        if (numSent == -1)
            return -1;
        if (numSent == 0)               /* Should never happen */
            fatal("sendfile: write() transferred 0 bytes");

        count -= numSent;
        totSent += numSent;
    }

    if (offset != NULL) {

        /* Return updated file offset in '*offset', and reset the file offset
           to the value it had when we were called. */

        *offset = lseek(in_fd, 0, SEEK_CUR);
        if (*offset == -1)
            return -1;
        if (lseek(in_fd, orig, SEEK_SET) == -1)
            return -1;
    }

    return totSent;
}
