/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 12-1 */

/* procfs_user_exe.c

   Demonstrate the use of /proc/PID files to obtain information about
   processes on a Linux system. In this case, given a username, this
   program scans all /proc/PID/status files to produce a list of all
   processes running under a particular real user ID; for each process,
   the process ID and command that it executes are shown.

   This program is Linux-specific.
*/
#define _GNU_SOURCE
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include "ugid_functions.h"
#include "tlpi_hdr.h"

#define MAX_LINE 1000

int
main(int argc, char *argv[])
{
    DIR *dirp;
    struct dirent *dp;
    char path[PATH_MAX];
    char line[MAX_LINE], cmd[MAX_LINE];
    FILE *fp;
    char *p;
    uid_t uid, checkedUid;
    Boolean gotName, gotUid;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s username\n", argv[0]);

    checkedUid = userIdFromName(argv[1]);
    if (checkedUid == -1)
        cmdLineErr("Bad username: %s\n", argv[1]);

    dirp = opendir("/proc");
    if (dirp == NULL)
        errExit("opendir");

    /* Scan entries under /proc directory */

    for (;;) {
        errno = 0;              /* To distinguish error from end-of-directory */
        dp = readdir(dirp);
        if (dp == NULL) {
            if (errno != 0)
                errExit("readdir");
            else
                break;
        }

        /* Since we are looking for /proc/PID directories, skip entries
           that are not directories, or don't begin with a digit. */

        if (dp->d_type != DT_DIR || !isdigit((unsigned char) dp->d_name[0]))
            continue;

        snprintf(path, PATH_MAX, "/proc/%s/status", dp->d_name);

        fp = fopen(path, "r");
        if (fp == NULL)
            continue;           /* Ignore errors: fopen() might fail if
                                   process has just terminated */

        gotName = FALSE;
        gotUid = FALSE;
        while (!gotName || !gotUid) {
            if (fgets(line, MAX_LINE, fp) == NULL)
                break;

            /* The "Name:" line contains the name of the command that
               this process is running */

            if (strncmp(line, "Name:", 5) == 0) {
                for (p = line + 5; *p != '\0' && isspace((unsigned char) *p); )
                    p++;
                strncpy(cmd, p, MAX_LINE - 1);
                cmd[MAX_LINE -1] = '\0';        /* Ensure null-terminated */

                gotName = TRUE;
            }

            /* The "Uid:" line contains the real, effective, saved set-,
               and file-system user IDs */

            if (strncmp(line, "Uid:", 4) == 0) {
                uid = strtol(line + 4, NULL, 10);
                gotUid = TRUE;
            }
        }

        fclose(fp);

        /* If we found a username and a UID, and the UID matches,
           then display the PID and command name */

        if (gotName && gotUid && uid == checkedUid)
            printf("%5s %s", dp->d_name, cmd);
    }

    exit(EXIT_SUCCESS);
}
