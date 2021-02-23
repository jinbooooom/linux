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

/* scm_cred.h

   Header file used by scm_cred_send.c and scm_cred_recv.c.
*/
#define _GNU_SOURCE             /* To get SCM_CREDENTIALS definition from
                                   <sys/socket.h> */
#include <sys/socket.h>
#include <sys/un.h>
#include "unix_sockets.h"       /* Declares our socket functions */
#include "tlpi_hdr.h"

#define SOCK_PATH "scm_cred"
