/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Header file for Listing 55-4 */

#ifndef CREATE_PID_FILE_H   /* Prevent accidental double inclusion */
#define CREATE_PID_FILE_H

#define CPF_CLOEXEC 1

int createPidFile(const char *progName, const char *pidFile, int flags);

#endif
