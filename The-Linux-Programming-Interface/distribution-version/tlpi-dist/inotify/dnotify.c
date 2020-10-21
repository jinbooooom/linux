/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 19 */

/* dnotify.c

   Demonstrate the use of the (obsolete) dnotify feature to obtain directory
   change notifications. (Modern programs should use inotify instead of
   dnotify. The inotify API is available in Linux 2.6.13 and later.)

   Usage is as shown in usageError() below. An example is the following:

        dnotify dir1:a xyz/dir2:acdM

   See also demo_inotify.c.

   This program is Linux-specific.
*/
#define _GNU_SOURCE             /* To get DN_* constants from <fcntl.h> */
#include <fcntl.h>
#include <signal.h>
#include "tlpi_hdr.h"

static void             /* Print (optional) message and usage info, then exit */
usageError(const char *progName, const char *msg)
{
    if (msg != NULL)
        fprintf(stderr, "%s", msg);

    fprintf(stderr, "Usage: %s directory:[events]...\n", progName);
    fprintf(stderr, "    Events are:\n"
        "        a - access; A - attrib; c - create; d - delete\n"
        "        m - modify; r - rename; M - multishot\n"
        "        (default is all events)\n");
    exit(EXIT_FAILURE);
}

static void
handler(int sig, siginfo_t *si, void *ucontext)
{
    printf("Got event on descriptor %d\n", si->si_fd);
                        /* UNSAFE (see Section 21.1.2) */
}

int
main(int argc, char *argv[])
{
    struct sigaction sa;
    int fd, events, fnum;
    const int NOTIFY_SIG = SIGRTMIN;
    char *p;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageError(argv[0], NULL);

    /* Establish handler for notification signal */

    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;           /* So handler gets siginfo_t arg. */
    if (sigaction(NOTIFY_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    for (fnum = 1; fnum < argc; fnum++) {
        p = strchr(argv[fnum], ':');    /* Look for optional ':' */

        if (p == NULL) {                /* Default is all events + multishot */
            events = DN_ACCESS | DN_ATTRIB | DN_CREATE | DN_DELETE |
                     DN_MODIFY | DN_RENAME | DN_MULTISHOT;
        } else {                        /* ':' present, parse event chars */
            *p = '\0';                  /* Terminates directory component */
            events = 0;
            for (p++; *p != '\0'; p++) {
                switch (*p) {
                case 'a': events |= DN_ACCESS;          break;
                case 'A': events |= DN_ATTRIB;          break;
                case 'c': events |= DN_CREATE;          break;
                case 'd': events |= DN_DELETE;          break;
                case 'm': events |= DN_MODIFY;          break;
                case 'r': events |= DN_RENAME;          break;
                case 'M': events |= DN_MULTISHOT;       break;
                default:  usageError(argv[0], "Bad event character\n");
                }
            }
        }

        /* Obtain a file descriptor for the directory to be monitored */

        fd = open(argv[fnum], O_RDONLY);
        if (fd == -1)
            errExit("open");
        printf("opened '%s' as file descriptor %d\n", argv[fnum], fd);

        /* Use alternate signal instead of SIGIO for dnotify events */

        if (fcntl(fd, F_SETSIG, NOTIFY_SIG) == -1)
            errExit("fcntl - F_SETSIG");

        /* Enable directory change notifications */

        if (fcntl(fd, F_NOTIFY, events) == -1)
            errExit("fcntl-F_NOTIFY");
        printf("events: %o\n", (unsigned int) events);
    }

    for (;;)
        pause();                        /* Wait for events */
}
