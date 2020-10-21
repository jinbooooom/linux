/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 18-2 */

/* bad_symlink.c

   The following code demonstrates a mistake in using symlink(): the link
   is created with an incorrect relative path, and the subsequent chmod()
   call fails. Note: symbolic links are interpreted relative to the directory
   in which they reside, not the current directory of the process.
*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd;

    if (mkdir("test", S_IRUSR | S_IWUSR | S_IXUSR) == -1)
        errExit("mkdir");
    if (chdir("test") == -1)
        errExit("chdir");
    fd = open("myfile", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");
    if (close(fd) == -1)
        errExit("close");
    if (symlink("myfile", "../mylink") == -1)
        errExit("symlink");
    if (chmod("../mylink", S_IRUSR) == -1)
        errExit("chmod");

    exit(EXIT_SUCCESS);
}
