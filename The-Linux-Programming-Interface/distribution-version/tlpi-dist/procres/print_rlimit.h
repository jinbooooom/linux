/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Header file for Listing 36-2 */

/* print_rlimit.h

   Header file for print_rlimit.c.
*/
#ifndef PRINT_RLIMIT_H      /* Prevent accidental double inclusion */
#define PRINT_RLIMIT_H

int printRlimit(const char *msg, int resource);

#endif
