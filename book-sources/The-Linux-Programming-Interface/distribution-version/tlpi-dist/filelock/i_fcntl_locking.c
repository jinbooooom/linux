/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 55-2 */

/* i_fcntl_locking.c

   Usage: i_fcntl_locking file...

   where 'file...' is a list of files on which to place locks - the user is
   then prompted to interactively enter commands to test/place locks on
   regions of the files.

   NOTE: The version of the program provided here is an enhanced version
   of that provided in the book. In particular, this version:

       1) handles multiple file name arguments, allowing locks to be
          applied to any of the named files,
       2) displays information about whether advisory or mandatory
          locking is in effect on each file, and
       3) allows the use of OFD locks, a type of file lock added in
          Linux 3.15.
*/
#define _GNU_SOURCE     /* To get definitions of 'OFD' locking commands */
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define MAX_LINE 100

#ifdef __linux__
#ifndef F_OFD_GETLK     /* In case we are on a system with glibc version
                           earlier than 2.20 */
#define F_OFD_GETLK     36
#define F_OFD_SETLK     37
#define F_OFD_SETLKW    38
#endif
#endif

/* Return TRUE if mandatory locking is enabled for fd. */

static Boolean
mandLockingEnabled(int fd)
{
    /* Mandatory locking is enabled for a file if the set-group-ID bit is on
       but group-execute permission is off (a combination that under earlier
       versions of UNIX had no useful meaning). If mandatory locking is enabled
       for a file, then attempts to perform write(2) or read(2) calls on locked
       regions of files will block until the lock is removed (or if the I/O
       call is nonblocking it will return immediately with an error). */

    struct stat sb;

    if (fstat(fd, &sb) == -1)
        errExit("stat");
    return (sb.st_mode & S_ISGID) != 0 && (sb.st_mode & S_IXGRP) == 0;
}

static void
displayCmdFmt(int argc, char *argv[], const int fdList[])
{
    int j;

    if (argc == 2) {            /* Only a single filename argument */
        printf("\nFormat: cmd lock start length [whence]\n\n");
    } else {
        printf("\nFormat: %scmd lock start length [whence]\n\n",
                (argc > 2) ? "file-num " : "");
        printf("    file-num is a number from the following list\n");
        for (j = 1; j < argc; j++)
            printf("        %2d  %-10s [%s locking]\n", j, argv[j],
                mandLockingEnabled(fdList[j]) ? "mandatory" : "advisory");
    }
    printf("    'cmd' is 'g' (GETLK), 's' (SETLK), or 'w' (SETLKW)\n");
#ifdef __linux__
    printf("        or for OFD locks: 'G' (OFD_GETLK), 'S' (OFD_SETLK), or "
            "'W' (OFD_SETLKW)\n");
#endif
    printf("    'lock' is 'r' (READ), 'w' (WRITE), or 'u' (UNLOCK)\n");
    printf("    'start' and 'length' specify byte range to lock\n");
    printf("    'whence' is 's' (SEEK_SET, default), 'c' (SEEK_CUR), "
           "or 'e' (SEEK_END)\n\n");
                /* Of course, SEEK_CUR is redundant since we can't
                   change the file offset in this program... */
}

int
main(int argc, char *argv[])
{
    int fd, numRead, cmd, status;
    char lock, cmdCh, whence, line[MAX_LINE];
    struct flock fl;
    long long len, st;
    int *fdList;
    int fileNum, j;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file...\n", argv[0]);

    fdList = calloc(argc, sizeof(int));
    if (fdList == NULL)
        errExit("calloc");

    for (j = 1; j < argc; j++) {
        fdList[j] = open(argv[j], O_RDWR);
        if (fdList[j] == -1)
            errExit("open (%s)", argv[j]);
    }

    /* Inform user what type of locking is in effect for each file. */

    printf("File       Locking\n");
    printf("----       -------\n");

    for (j = 1; j < argc; j++)
        printf("%-10s %s\n", argv[j], mandLockingEnabled(fdList[j]) ?
                "mandatory" : "advisory");
    printf("\n");

    printf("Enter ? for help\n");

    for (;;) {          /* Prompt for locking command and carry it out */
        printf("PID=%ld> ", (long) getpid());
        fflush(stdout);

        if (fgets(line, MAX_LINE, stdin) == NULL)       /* EOF */
            exit(EXIT_SUCCESS);
        line[strlen(line) - 1] = '\0';          /* Remove trailing '\n' */

        if (*line == '\0')
            continue;                           /* Skip blank lines */

        if (line[0] == '?') {
            displayCmdFmt(argc, argv, fdList);
            continue;
        }

        whence = 's';                   /* In case not otherwise filled in */

        if (argc == 2) {                /* Just 1 file arg on command line? */
            fileNum = 1;                /* Then no need to read a file number */
            numRead = sscanf(line, " %c %c %lld %lld %c",
                    &cmdCh, &lock, &st, &len, &whence);
        } else {
            numRead = sscanf(line, "%d %c %c %lld %lld %c",
                    &fileNum, &cmdCh, &lock, &st, &len, &whence);
        }

        fl.l_start = st;
        fl.l_len = len;

        if (fileNum < 1 || fileNum >= argc) {
            printf("File number must be in range 1 to %d\n", argc-1);
            continue;
        }

        fd = fdList[fileNum];

        if (!((numRead >= 4 && argc == 2) || (numRead >= 5 && argc > 2)) ||
                strchr("gswGSW", cmdCh) == NULL ||
                strchr("rwu", lock) == NULL || strchr("sce", whence) == NULL) {
            printf("Invalid command!\n");
            continue;
        }

        cmd =
#ifdef __linux__
              (cmdCh == 'G') ? F_OFD_GETLK : (cmdCh == 'S') ? F_OFD_SETLK :
              (cmdCh == 'W') ? F_OFD_SETLKW :
#endif
              (cmdCh == 'g') ? F_GETLK : (cmdCh == 's') ? F_SETLK : F_SETLKW;
#ifdef __linux__
        fl.l_pid = 0;   /* Required for 'OFD' locking commands */
#endif
        fl.l_type = (lock == 'r') ? F_RDLCK : (lock == 'w') ? F_WRLCK : F_UNLCK;
        fl.l_whence = (whence == 'c') ? SEEK_CUR :
                      (whence == 'e') ? SEEK_END : SEEK_SET;

        status = fcntl(fd, cmd, &fl);           /* Perform request... */

        if (cmd == F_GETLK
#ifdef __linux__
                || cmd == F_OFD_GETLK
#endif
                ) {
            if (status == -1) {
                errMsg("fcntl");
            } else {
                if (fl.l_type == F_UNLCK)
                    printf("[PID=%ld] Lock can be placed\n", (long) getpid());
                else                            /* Locked out by someone else */
                    printf("[PID=%ld] Denied by %s lock on %lld:%lld "
                            "(held by PID %ld)\n", (long) getpid(),
                            (fl.l_type == F_RDLCK) ? "READ" : "WRITE",
                            (long long) fl.l_start,
                            (long long) fl.l_len, (long) fl.l_pid);
            }
        } else {
            if (status == 0)
                printf("[PID=%ld] %s\n", (long) getpid(),
                        (lock == 'u') ? "unlocked" : "got lock");
            else if (errno == EAGAIN || errno == EACCES)
                printf("[PID=%ld] failed (incompatible lock)\n",
                        (long) getpid());
            else if (errno == EDEADLK)                          /* F_SETLKW */
                printf("[PID=%ld] failed (deadlock)\n", (long) getpid());
            else
                errMsg("fcntl");
        }
    }
}
