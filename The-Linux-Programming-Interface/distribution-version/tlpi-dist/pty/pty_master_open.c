/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Listing 64-1 */

/* pty_master_open.c

   Implement our ptyMasterOpen() function, based on UNIX 98 pseudoterminals.
   See comments below.

   See also pty_master_open_bsd.c.
*/
#if ! defined(__sun)
        /* Prevents ptsname() declaration being visible on Solaris 8 */
#if ! defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 600
#define _XOPEN_SOURCE 600
#endif
#endif
#include <stdlib.h>
#include <fcntl.h>
#include "pty_master_open.h"            /* Declares ptyMasterOpen() */
#include "tlpi_hdr.h"

/* Some implementations don't have posix_openpt() */

#if defined(__sun)                      /* Not on Solaris 8 */
#define NO_POSIX_OPENPT
#endif

#ifdef NO_POSIX_OPENPT

static int
posix_openpt(int flags)
{
    return open("/dev/ptmx", flags);
}

#endif

/* Open a pty master, returning file descriptor, or -1 on error.
   On successful completion, the name of the corresponding pty
   slave is returned in 'slaveName'. 'snLen' should be set to
   indicate the size of the buffer pointed to by 'slaveName'. */

int
ptyMasterOpen(char *slaveName, size_t snLen)
{
    int masterFd, savedErrno;
    char *p;

    masterFd = posix_openpt(O_RDWR | O_NOCTTY);         /* Open pty master */
    if (masterFd == -1)
        return -1;

    if (grantpt(masterFd) == -1) {              /* Grant access to slave pty */
        savedErrno = errno;
        close(masterFd);                        /* Might change 'errno' */
        errno = savedErrno;
        return -1;
    }

    if (unlockpt(masterFd) == -1) {             /* Unlock slave pty */
        savedErrno = errno;
        close(masterFd);                        /* Might change 'errno' */
        errno = savedErrno;
        return -1;
    }

    p = ptsname(masterFd);                      /* Get slave pty name */
    if (p == NULL) {
        savedErrno = errno;
        close(masterFd);                        /* Might change 'errno' */
        errno = savedErrno;
        return -1;
    }

    if (strlen(p) < snLen) {
        strncpy(slaveName, p, snLen);
    } else {                    /* Return an error if buffer too small */
        close(masterFd);
        errno = EOVERFLOW;
        return -1;
    }

    return masterFd;
}
