/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Supplementary program for Chapter 3 */

#ifndef ALT_FUNCTIONS_H
#define ALT_FUNCTIONS_H         /* Prevent accidental double inclusion */

#if defined(__osf__) || defined(__hpux) || defined(_AIX) || \
        defined(__sgi) || defined(__APPLE__)
#define strsignal(sig) ALT_strsignal(sig)
#endif
char *ALT_strsignal(int sig);

#if defined(__hpux) || defined(__osf__)
#define hstrerror(err) ALT_hstrerror(err)
#endif
char *ALT_hstrerror(int sig);

#if defined(__hpux) || defined(__osf__)
#define posix_openpt(flags) ALT_posix_openpt(flags)
#endif
int ALT_posix_openpt(int flags);

#endif
