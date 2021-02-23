/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 5-1 */

/* bad_exclusive_open.c

   The following code shows why we need the open() O_EXCL flag.

   This program to tries ensure that it is the one that creates the file
   named in its command-line argument. It does this by trying to open()
   the filename once without the O_CREAT flag (if this open() succeeds
   then the program know it is not the creator of the file), and if
   that open() fails, it calls open() a second time, with the O_CREAT flag.

   If the first open() fails, the program assumes that it is the creator
   of the file. However this may not be true: some other process may have
   created the file between the two calls to open().
*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);

    fd = open(argv[1], O_WRONLY);       /* Open 1: check if file exists */
    if (fd != -1) {                     /* Open succeeded */
        printf("[PID %ld] File \"%s\" already exists\n",
                (long) getpid(), argv[1]);
        close(fd);
    } else {
        if (errno != ENOENT) {          /* Failed for unexpected reason */
            errExit("open");
        } else {
            printf("[PID %ld] File \"%s\" doesn't exist yet\n",
                    (long) getpid(), argv[1]);
            if (argc > 2) {             /* Delay between check and create */
                sleep(5);               /* Suspend execution for 5 seconds */
                printf("[PID %ld] Done sleeping\n", (long) getpid());
            }
            fd = open(argv[1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            if (fd == -1)
                errExit("open");

            /*FIXME: should use %zd here, and remove (long) cast */
            printf("[PID %ld] Created file \"%s\" exclusively\n",
                    (long) getpid(), argv[1]);          /* MAY NOT BE TRUE! */
        }
    }

    exit(EXIT_SUCCESS);
}
