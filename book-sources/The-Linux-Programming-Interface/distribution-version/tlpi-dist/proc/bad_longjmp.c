/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 6-2 */

/* bad_longjmp.c

   Demonstrate the incorrect use of longjmp() to jump into a function
   that has already returned.

   Usage: bad_longjmp [x]

   The presence or absence of the command-line argument determines
   whether we will call an intervening recursive function between the
   function that establishes the jump point, and the one that does
   the jump. Each case results in a different erroneous behaviour.
*/
#include <setjmp.h>
#include "tlpi_hdr.h"

static jmp_buf env;     /* Global buffer for saving environment */

static void
doJump(void)
{
    printf("Entered doJump\n");
    longjmp(env, 2);
    printf("Exiting doJump\n");
}

static void
setJump2(void)
{
    printf("Entered setJump2\n");
    setjmp(env);
    printf("Exiting setJump2\n");
}

static void
setJump(void)
{

    printf("Entered setJump\n");
    setJump2();
    printf("Exiting setJump\n");
}

static void
recur(int n)
{
    printf("Entered recur %d\n", n);
    if (n > 0)
        recur(n - 1);
    printf("Exiting recur %d\n", n);
}

int
main(int argc, char *argv[])
{
    setJump();
    if (argc > 1)
        recur(2);
    doJump();
    printf("Back at main\n");

    exit(EXIT_SUCCESS);
}
