/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 60-2 */

/* is_echo_v2_sv.c

   An implementation of the TCP "echo" service that can either be invoked from
   the command line, or via inetd(8) if we specify the "-i" command-line option.

   If run from the command line, this program must be run under a root login,
   in order to allow the "echo" port (7) to be bound. Alternatively, for test
   purposes, you can replace the SERVICE name below with a suitable unreserved
   port number (e.g., "51000"), and make a corresponding change in the client.

   If run from inetd(8), place a line similar to the following in
   /etc/inetd.conf (you will need to modify <some-path> as required):

        echo stream tcp nowait root /some-path/is_echo_v2_sv is_echo_v2_sv -i

   Note that this program is very similar to is_echo_sv.c: all we've
   done is add a few extra lines of code to handle the "-i" option.

   See also is_echo_sv.c.
*/
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "become_daemon.h"
#include "inet_sockets.h"       /* Declares our socket functions */
#include "tlpi_hdr.h"

#define SERVICE "echo"          /* Name of TCP service */
#define BUF_SIZE 4096

static void             /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig)
{
    int savedErrno;             /* Save 'errno' in case changed here */

    savedErrno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

static void             /* Handle client: copy socket input back to socket */
handleRequest(int cfd)
{
    ssize_t numRead;
    char buf[BUF_SIZE];

    while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
        if (write(cfd, buf, numRead) != numRead) {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        syslog(LOG_ERR, "Error from read(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char *argv[])
{
    int lfd, cfd;               /* Listening and connected sockets */
    struct sigaction sa;

    /* The "-i" option means we were invoked from inetd(8), so that
       all we need to do is handle the connection on STDIN_FILENO */

    if (argc > 1 && strcmp(argv[1], "-i") == 0) {
        handleRequest(STDIN_FILENO);
        exit(EXIT_SUCCESS);
    }

    if (becomeDaemon(0) == -1)
        errExit("becomeDaemon");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    lfd = inetListen(SERVICE, 10, NULL);
    if (lfd == -1) {
        syslog(LOG_ERR, "Could not create server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;) {
        cfd = accept(lfd, NULL, NULL);  /* Wait for connection */
        if (cfd == -1) {
            syslog(LOG_ERR, "Failure in accept(): %s",
                    strerror(errno));
            continue;           /* Try next */
        }

        switch (fork()) {       /* Create child for each client */
        case -1:
            syslog(LOG_ERR, "Can't create child (%s)",
                    strerror(errno));
            close(cfd);         /* Give up on this client */
            break;              /* May be temporary; try next client */

        case 0:                 /* Child */
            close(lfd);         /* Don't need copy of listening socket */
            handleRequest(cfd);
            _exit(EXIT_SUCCESS);

        default:                /* Parent */
            close(cfd);         /* Don't need copy of connected socket */
            break;              /* Loop to accept next connection */
        }
    }
}
