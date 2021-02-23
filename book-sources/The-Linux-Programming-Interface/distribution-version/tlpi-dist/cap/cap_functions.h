/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Supplementary program for Chapter 39 */

/* cap_functions.h

   Useful functions for working with capabilities.
*/

#ifndef CAP_FUNCTIONS_H             /* Prevent double inclusion */
#define CAP_FUNCTIONS_H

#include <stdio.h>
#include <sys/capability.h>
#include <stdbool.h>

/* Change the 'setting' of the specified 'capability' in the capability set
   specified by 'flag'.

   'flag' is one of CAP_PERMITTED, CAP_EFFECTIVE, or CAP_INHERITABLE.
   'setting' is one of CAP_SET or CAP_CLEAR.

   Returns: 0 on success or -1 on error. */

int modifyCapSetting(cap_flag_t flag, int capability, int setting);

/* Display a securebits mask in either short or long form, depending on
   the value of 'verbose'. */

void printSecbits(int secbits, bool verbose, FILE *fp);

#endif
