/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 8 */

/* t_getpwnam_r.c

   Demonstrate the use of getpwnam_r() to retrieve the password record for
   a named user from the system password file.
*/
#include <pwd.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct passwd pwd;
    struct passwd *result;
    char *buf;
    size_t bufSize;
    int s;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s username\n", argv[0]);

    bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    buf = malloc(bufSize);
    if (buf == NULL)
        errExit("malloc %d", bufSize);

    s = getpwnam_r(argv[1], &pwd, buf, bufSize, &result);
    if (s != 0)
        errExitEN(s, "getpwnam_r");

    if (result != NULL)
        printf("Name: %s\n", pwd.pw_gecos);
    else
        printf("Not found\n");

    exit(EXIT_SUCCESS);
}
