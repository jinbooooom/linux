/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 64-7 */

/* unbuffer.c

   Use a pseudoterminal to circumvent the block buffering performed by the
   stdio library when standard output is redirected to a file or pipe.
   When a program is started using 'unbuffer', its output is redirected to
   a pseudoterminal, and is consequently line-buffered.

   Usage: unbuffer prog [arg ...]
*/
#include <termios.h>
#if ! defined(__hpux)
/* HP-UX 11 doesn't have this header file */
#include <sys/select.h>
#endif
#include "pty_fork.h"           /* Declaration of ptyFork() */
#include "tty_functions.h"      /* Declaration of ttySetRaw() */
#include "tlpi_hdr.h"

#define BUF_SIZE 256
#define MAX_SNAME 1000

struct termios ttyOrig;

static void             /* Reset terminal mode on program exit */
ttyReset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
        errExit("tcsetattr");
}

int
main(int argc, char *argv[])
{
    char slaveName[MAX_SNAME];
    int masterFd;
    fd_set inFds;
    char buf[BUF_SIZE];
    ssize_t numRead;
    struct winsize ws;

    /* Retrieve the attributes of terminal on which we are started */

    if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
        errExit("tcgetattr");
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &ws) < 0)
        errExit("TIOCGWINSZ error");

    /* Create a child process, with parent and child connected via a
       pty pair. The child is connected to the pty slave and its terminal
       attributes are set to be the same as those retrieved above. */

    switch (ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws)) {
    case -1:
        errExit("ptyFork");

    case 0:     /* Child executes command given in argv[1]... */
        execvp(argv[1], &argv[1]);
        errExit("execvp");

    default:    /* Parent relays data between terminal and pty master */

        /* Place terminal in raw mode so that we can pass all
           terminal input to the pseudoterminal master untouched */

        ttySetRaw(STDIN_FILENO, &ttyOrig);
        if (atexit(ttyReset) != 0)
            errExit("atexit");

        /* Loop monitoring terminal and pty master for input. If the
           terminal is ready for input, read some bytes and write
           them to the pty master. If the pty master is ready for
           input, read some bytes and write them to the terminal. */

        for (;;) {
            FD_ZERO(&inFds);
            FD_SET(STDIN_FILENO, &inFds);
            FD_SET(masterFd, &inFds);
            if (select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1)
                errExit("select");

            if (FD_ISSET(STDIN_FILENO, &inFds)) {
                numRead = read(STDIN_FILENO, buf, BUF_SIZE);
                if (numRead <= 0)
                    exit(EXIT_SUCCESS);

                if (write(masterFd, buf, numRead) != numRead)
                    fatal("partial/failed write (masterFd)");
            }

            if (FD_ISSET(masterFd, &inFds)) {
                numRead = read(masterFd, buf, BUF_SIZE);
                if (numRead <= 0)
                    exit(EXIT_SUCCESS);

                if (write(STDOUT_FILENO, buf, numRead) != numRead)
                    fatal("partial/failed write (STDOUT_FILENO)");
            }
        }
    }
}
