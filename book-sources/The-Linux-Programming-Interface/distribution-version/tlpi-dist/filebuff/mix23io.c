/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 13 */

/* mix23io.c

   Illustrates the impact of stdio buffering when using stdio library functions
   and I/O system calls to work on the same file.

   Try running this program (with stdout directed to the terminal) without and
   with a command-line argument (any string).
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
    printf("To man the world is twofold, ");
    if (argc > 1)
        printf("\n");
    write(STDOUT_FILENO, "in accordance with his twofold attitude.\n", 41);

    exit(EXIT_SUCCESS);
}
