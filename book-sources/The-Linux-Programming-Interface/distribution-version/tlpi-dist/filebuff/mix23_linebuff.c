/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 13-4 */

/* mix23_linebuff.c

   Illustrates the impact of stdio buffering when using stdio library
   functions and I/O system calls to work on the same file. Observe the
   difference in output when running this program with output directed
   to a terminal and again with output directed to a file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
    printf("If I had more time, \n");
    write(STDOUT_FILENO, "I would have written you a shorter letter.\n", 43);
    exit(EXIT_SUCCESS);
}
