/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Header file for Listing 23-6 */

#ifndef ITIMERSPEC_FROM_STR_H
#define ITIMERSPEC_FROM_STR_H

#include <time.h>

void itimerspecFromStr(char *str, struct itimerspec *tsp);

#endif
