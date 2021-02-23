/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 39 */

/* cap_launcher.c

   Launch a program with the credentials (UIDs, GIDs, supplementary GIDs)
   of a specified user, and with the capabilities specified on the
   command line.  The program relies on the use of ambient capabilities,
   a feature that first appeared in Linux 4.3.
*/
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <linux/securebits.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "cap_functions.h"
#include "tlpi_hdr.h"

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s user cap,... cmd arg...\n", pname);
    fprintf(stderr, "\t'user' is the user with whose credentials\n");
    fprintf(stderr, "\t\tthe program is to be launched\n");
    fprintf(stderr, "\t'cap,...' is the set of capabilities with which\n");
    fprintf(stderr, "\t\tthe program is to be launched\n");
    fprintf(stderr, "\t'cmd' and 'arg...' specify the command plus\n");
    fprintf(stderr, "\t\tfor the program that is to be launched\n");
    exit(EXIT_FAILURE);
}

/* Switch credentials (user ID, group ID, supplementary groups) to
   those for the user named in 'user' */

static void
setCredentials(char *user)
{
    struct passwd *pwd;
    int ngroups;
    gid_t *groups;

    /* Look up user in user database */

    pwd = getpwnam(user);
    if (pwd == NULL) {
        fprintf(stderr, "Unknown user: %s\n", user);
        exit(EXIT_FAILURE);
    }

    /* Find out how many supplementary groups the user is a member of */

    ngroups = 0;
    getgrouplist(user, pwd->pw_gid, NULL, &ngroups);

    /* Allocate an array for supplementary group IDs */

    groups = calloc(ngroups, sizeof(gid_t));
    if (groups == NULL)
        errExit("calloc");

    /* Get supplementary group list of 'user' from group database */

    if (getgrouplist(user, pwd->pw_gid, groups, &ngroups) == -1)
        errExit("getgrouplist");

    /* Set the supplementary group list */

    if (setgroups(ngroups, groups) == -1)
        errExit("setgroups");

    /* Set all group IDs to GID of this user */

    if (setresgid(pwd->pw_gid, pwd->pw_gid, pwd->pw_gid) == -1)
        errExit("setresgid");

    /* Set all user IDs to UID of this user */

    if (setresuid(pwd->pw_uid, pwd->pw_uid, pwd->pw_uid) == -1)
        errExit("setresuid");
}

/* Add a set of capabilities to the process's ambient list */

static void
setAmbientCapabilities(char *capList)
{
    cap_value_t cap;

    /* Walk through the capabilities listed in the comma-delimited list
       of capability names in 'capList', adding each capability to the
       ambient set. This will cause the capability to pass into the
       process permitted and effective sets during exec(). */

    for (char *p = capList; (p = strtok(p, ",")); p = NULL) {

        /* Convert the capability name to a capability number */

        if (cap_from_name(p, &cap) == -1) {
            fprintf(stderr, "Unrecognized capability name: %s\n", p);
            exit(EXIT_FAILURE);
        }

        /* In order to place a capability into the ambient set,
           that capability must also be in the inheritable set */

        if (modifyCapSetting(CAP_INHERITABLE, cap, CAP_SET) == -1) {
            fprintf(stderr, "Could not raise '%s' inheritable "
                    "capability (%s)\n", p, strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* Raise the capability in the ambient set */

        if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) == -1) {
            fprintf(stderr, "Could not raise '%s' ambient "
                    "capability (%s)\n", p, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

int
main(int argc, char *argv[])
{
    if (argc < 4 || strcmp(argv[1], "--help") == 0)
        usage(argv[0]);

    if (geteuid() != 0)
        fatal("Must be run as root");

    /* Set "no setuid fixup" securebit so that when we switch to
       a nonzero UID, we don't lose capabilities */

    if (prctl(PR_SET_SECUREBITS, SECBIT_NO_SETUID_FIXUP, 0, 0, 0) == -1)
        errExit("prctl");

    setCredentials(argv[1]);

    setAmbientCapabilities(argv[2]);

    /* Execute the program (with arguments) named in argv[3]... */

    execvp(argv[3], &argv[3]);
    errExit("execvp");
}
