/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 59-2:a */

/* is_seqnum_v2.h

   Header file for is_seqnum_v2_sv.c and is_seqnum_v2_cl.c.
*/
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include "inet_sockets.h"       /* Declares our socket functions */
#include "read_line.h"          /* Declaration of readLine() */
#include "tlpi_hdr.h"

#define PORT_NUM_STR "50000"    /* Port number for server */

#define INT_LEN 30              /* Size of string able to hold largest
                                   integer (including terminating '\n') */
