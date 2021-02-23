/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

#include <stdlib.h>
#include <stdio.h>

void
xyz(void)
{
    printf("        main-xyz\n");
}

int
main(int argc, char*argv[])
{
    void func1(void), func2(void), func3(void);

    func1();
    func2();

    exit(EXIT_SUCCESS);
}
