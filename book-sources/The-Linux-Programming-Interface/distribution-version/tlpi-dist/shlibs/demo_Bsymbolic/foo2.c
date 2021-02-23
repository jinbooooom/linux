/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* foo2.c

*/
#include <stdlib.h>
#include <stdio.h>

void
xyz(void)
{
    printf("        func2-xyz\n");
}

void
abc(void)
{
    printf("        func1-abc\n");
}

void
func2(int x)
{
    printf("Called func2\n");
    xyz();
    abc();
}
