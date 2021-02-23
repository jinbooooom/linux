/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Solution for Exercise 59-1:b */

/* read_line_buf.h

   Header file for read_line_buf.c (implementation of readLineBuf()).
*/
#ifndef READ_LINE_BUF_H         /* Prevent accidental double inclusion */
#define READ_LINE_BUF_H

#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define RL_MAX_BUF 10

struct ReadLineBuf {
    int     fd;                 /* File descriptor from which to read */
    char    buf[RL_MAX_BUF];    /* Current buffer from file */
    int     next;               /* Index of next unread character in 'buf' */
    ssize_t len;                /* Number of characters in 'buf' */
};

void readLineBufInit(int fd, struct ReadLineBuf *rlbuf);

ssize_t readLineBuf(struct ReadLineBuf *rlbuf, char *buffer, size_t n);

#endif
