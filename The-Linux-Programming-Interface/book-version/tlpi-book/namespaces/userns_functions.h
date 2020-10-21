/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Supplementary program for Chapter Z */

/* userns_functions.h

   Some useful auxiliary functions when working with user namespaces.
*/
#ifndef USERNS_FUNCTIONS_H          /* Prevent double inclusion */
#define USERNS_FUNCTIONS_H

#include <unistd.h>

void display_creds_and_caps(char *str);

int update_map(char *mapping, char *map_file);

int proc_setgroups_write(pid_t child_pid, char *str);

#endif
