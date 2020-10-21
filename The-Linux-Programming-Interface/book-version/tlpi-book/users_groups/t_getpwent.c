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

#include <pwd.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct passwd *pwd;

    while ((pwd = getpwent()) != NULL)
        printf("%-8s %5ld\n", pwd->pw_name, (long) pwd->pw_uid);
    endpwent();
    exit(EXIT_SUCCESS);
}
