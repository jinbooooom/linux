/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Supplementary program for Chapter Z */

/* userns_functions.c

   Some useful auxiliary functions when working with user namespaces.
*/
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/capability.h>
#include "userns_functions.h"
#include "tlpi_hdr.h"

/* Display calling process's (effective) credentials and capabilities */

void
display_creds_and_caps(char *str)
{
    cap_t caps;
    char *s;

    printf("%seUID = %ld; eGID=%ld;  ", str,
            (long) geteuid(), (long) getegid());

    caps = cap_get_proc();
    if (caps == NULL)
        errExit("cap_get_proc");

    s = cap_to_text(caps, NULL);
    if (s == NULL)
        errExit("cap_to_text");
    printf("capabilities: %s\n", s);

    cap_free(caps);
    cap_free(s);
}

/* Update the mapping file 'map_file', with the value provided in
   'mapping', a string that defines a UID or GID mapping. A UID or
   GID mapping consists of one or more newline-delimited records
   of the form:

       ID_inside-ns    ID-outside-ns   length

   Requiring the user to supply a string that contains newlines is
   of course inconvenient for command-line use. Thus, we permit the
   use of commas to delimit records in this string, and replace them
   with newlines before writing the string to the file.

   Returns: 0 on success; -1 on error. */

int
update_map(char *mapping, char *map_file)
{
    int fd;
    size_t map_len;     /* Length of 'mapping' */
    int status;

    /* Replace commas in mapping string with newlines */

    map_len = strlen(mapping);
    for (int j = 0; j < map_len; j++)
        if (mapping[j] == ',')
            mapping[j] = '\n';

    fd = open(map_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: open %s: %s\n", map_file, strerror(errno));
        return -1;
    }

    status = 0;

    if (write(fd, mapping, map_len) != map_len) {
        fprintf(stderr, "ERROR: writing to %s: %s\n",
                map_file, strerror(errno));
        status = -1;
    }

    close(fd);
    return status;
}

/* Linux 3.19 made a change in the handling of setgroups(2) and the
   'gid_map' file to address a security issue. The issue allowed
   *unprivileged* users to employ user namespaces in order to drop groups
   from their supplementary group list using setgroups(2).  (Formerly, this
   possibility was available only to privileged processes.) The effect was to
   create possibilities for unprivileged process to access files for which
   they would not otherwise have had permission. (For further details, see
   the user_namespaces(7) man page.)

   The upshot of the 3.19 changes is that in order for a process lacking
   suitable privileges (i.e., one that lacks the CAP_SETGID capability in
   the parent user namespace) to update the 'gid_maps' file, use of the
   setgroups() system call in this user namespace must first be disabled
   by writing "deny" to one of the /proc/PID/setgroups files for this
   namespace.  That is the purpose of the following function.

   Returns: 0 on success; -1 on error. */

int
proc_setgroups_write(pid_t child_pid, char *str)
{
    char setgroups_path[PATH_MAX];
    int fd;
    int status;

    snprintf(setgroups_path, PATH_MAX, "/proc/%ld/setgroups",
            (long) child_pid);

    fd = open(setgroups_path, O_RDWR);
    if (fd == -1) {

        /* We may be on a system that doesn't support /proc/PID/setgroups.
           In that case, the file won't exist, and the system won't impose
           the restrictions that Linux 3.19 added. That's fine: we don't
           need to do anything in order to permit 'gid_map' to be updated.

           However, if the error from open() was something other than the
           ENOENT error that is expected for that case, let the user know. */

        if (errno == ENOENT) {
            return 0;
        } else {
            fprintf(stderr, "ERROR: open %s: %s\n", setgroups_path,
                strerror(errno));
            return -1;
        }
    }

    status = 0;

    if (write(fd, str, strlen(str)) == -1) {
        fprintf(stderr, "ERROR: writing to %s: %s\n", setgroups_path,
            strerror(errno));
        status = -1;
    }

    close(fd);
    return status;
}
