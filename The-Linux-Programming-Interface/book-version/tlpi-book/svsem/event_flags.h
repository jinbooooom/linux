/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Solution for Exercise 47-5:b */

#ifndef EVENT_FLAGS_H
#define EVENT_FLAGS_H           /* Prevent accidental double inclusion */

#include "tlpi_hdr.h"

int waitForEventFlag(int semId, int semNum);

int clearEventFlag(int semId, int semNum);

int setEventFlag(int semId, int semNum);

int getFlagState(int semId, int semNum, Boolean *isSet);

#endif
