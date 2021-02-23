/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Solution for Exercise 36-2:c */

#ifndef PRINT_RUSAGE_H      /* Prevent accidental double inclusion */
#define PRINT_RUSAGE_H

#include <sys/resource.h>

void printRusage(const char *leader, const struct rusage *ru);

#endif
