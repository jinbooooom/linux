/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter Z */

/* ns_capable.c

   Test whether a process (identified by PID) might--subject to LSM (Linux
   Security Module) checks--have capabilities in a namespace (identified by
   a /proc/PID/ns/xxx file).

   Usage: ./ns_capable <pid> <namespace-file>
*/
#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/capability.h>

#ifndef NS_GET_USERNS
#define NSIO    0xb7
#define NS_GET_USERNS           _IO(NSIO, 0x1)
#define NS_GET_PARENT           _IO(NSIO, 0x2)
#define NS_GET_NSTYPE           _IO(NSIO, 0x3)
#define NS_GET_OWNER_UID        _IO(NSIO, 0x4)
#endif

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

#define fatal(msg)      do { fprintf(stderr, "%s\n", msg); \
                             exit(EXIT_FAILURE); } while (0)

/* Display capabilities of process with specified PID */

static void
display_process_capabilities(pid_t pid)
{
    cap_t caps;
    char *cap_string;

    caps = cap_get_pid(pid);
    if (caps == NULL)
        errExit("cap_get_proc");

    cap_string = cap_to_text(caps, NULL);
    if (cap_string == NULL)
        errExit("cap_to_text");

    printf("Capabilities: %s\n", cap_string);

    cap_free(caps);
    cap_free(cap_string);
}

/* Obtain the effective UID of the process 'pid' by
   scanning its /proc/PID/status file */

static uid_t
euid_of_process(pid_t pid)
{
    char path[PATH_MAX];
    char line[1024];
    int uid;
    FILE *fp;

    snprintf(path, sizeof(path), "/proc/%ld/status", (long) pid);

    fp = fopen(path, "r");
    if (fp == NULL)
        errExit("fopen-/proc/PID/status");

    for (;;) {
        if (fgets(line, sizeof(line), fp) == NULL) {

            /* We reached EOF without finding "Uid:" record (should never
               happen) */

            fprintf(stderr, "Failure scanning for 'Uid:' in %s\n", path);
            exit(EXIT_FAILURE);
        }

        if (strstr(line, "Uid:") == line) {
            sscanf(line, "Uid: %*d %d %*d %*d", &uid);
            fclose(fp);
            return uid;
        }
    }
}

/* Return true if two file descriptors refer to the same namespace,
   otherwise false */

static bool
ns_equal(int nsfd1, int nsfd2)
{
    struct stat sb1, sb2;

    if (fstat(nsfd1, &sb1) == -1)
        errExit("fstat-nsfd1");
    if (fstat(nsfd2, &sb2) == -1)
        errExit("fstat-nsfd2");

    /* Namespaces are equal if *both* the device ID and the inode number
       in the 'stat' records match */

    return sb1.st_dev == sb2.st_dev && sb1.st_ino == sb2.st_ino;
}

/* Return the type of the namespace referred to by 'ns_fd' */

static int
ns_type(int ns_fd)
{
    int nstype = ioctl(ns_fd, NS_GET_NSTYPE);
    if (nstype == -1)
        errExit("ioctl-NS_GET_NSTYPE");

    return nstype;
}

/* Return a file descriptor for the user namespace that owns the namespace
   referred to by 'ns_fd' */

static int
owning_userns_of(int ns_fd)
{
    int userns_fd = ioctl(ns_fd, NS_GET_USERNS);
    if (userns_fd == -1)
        errExit("ioctl-NS_GET_USERNS");

    return userns_fd;
}

/* Return the UID of the creator of the namespace referred to by 'userns_fd' */

static int
uid_of_userns_owner(int userns_fd)
{
    uid_t owner_uid;

    if (ioctl(userns_fd, NS_GET_OWNER_UID, &owner_uid) == -1) {
        perror("ioctl-NS_GET_OWNER_UID");
        exit(EXIT_FAILURE);
    }

    return owner_uid;
}

/* Determine whether 'fd_x' refers to an ancestor user namespace of the
   user namespace referred to by 'fd_y'.

   Returns: -1 if 'fd_x' does not refer to an ancestor user namespace;
   otherwise, if 'fd_x' does refer to an ancestor user namespace, then a
   file descriptor (an value >= 0) that refers to the user namespace
   that is the immediate descendant of 'fd_x' in the chain of user
   namespaces from 'fd_x' to 'fd_y'. */

static int
is_ancestor_userns(int fd_x, int fd_y)
{
    int parent, child;  /* File descriptors that refer to namespaces */

    /* Starting at the parent of the user namespace referred to by
       'fd_y', we walk upward through the chain of ancestor namespaces
       until we can traverse no further, or until we find a namespace
       that is the same as the one referred to by 'fd_x'. */

    child = fd_y;

    for (;;) {
        parent = ioctl(child, NS_GET_PARENT);

        if (parent == -1) {

            /* The error here should be EPERM, meaning no parent of this
               user namespace (because it is the initial namespace). Any
               other error is unexpected, and we terminate. */

            if (errno != EPERM)
                errExit("ioctl-NS_GET_PARENT");

            /* We traversed as far as we could, and did not find 'fd_x' in
               the chain of ancestors of 'fd_y'. */

            return -1;
        }

        /* If 'parent' and 'fd_x' are the same namespace, then we need
           traverse no further in the series of user namespace ancestors:
           'fd_x' does refer to an ancestor of 'fd_y'. */

        if (ns_equal(parent, fd_x)) {
            close(parent);              /* No longer need this FD */
            return child;
        }

        /* Otherwise, check the next ancestor user namespace */

        close(child);
        child = parent;
    }
}

int
main(int argc, char *argv[])
{
    char *pid_str;      /* PID from command line */
    pid_t pid;          /* That PID converted to numeric form */
    int target_ns;      /* FD referring to target NS (from command line) */
    int target_userns;  /* FD referring to user NS that owns 'target_ns' */
    int pid_userns;     /* FD referring to user NS of PID in command line */
    char path[PATH_MAX];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s PID ns-file\n", argv[0]);
        fprintf(stderr, "\t'ns-file' is a /proc/PID/ns/xxxx file\n");
        exit(EXIT_FAILURE);
    }

    pid_str = argv[1];
    pid = atoi(pid_str);

    /* Obtain a file descriptor that refers to the target user namespace */

    target_ns = open(argv[2], O_RDONLY);
    if (target_ns == -1)
        errExit("open-ns-file");

    /* In order to determine whether the process has capabilities in the
       specified namespace, we must determine the relevant user namespace,
       which is 'target_ns' itself if 'target_ns' refers to a user namespace,
       otherwise the user namespace that owns 'target_ns' */

    if (ns_type(target_ns) == CLONE_NEWUSER) {
        target_userns = target_ns;
    } else {
        target_userns = owning_userns_of(target_ns);
        close(target_ns);               /* No longer need this FD */
    }

    /* Obtain a file descriptor for the user namespace of the PID */

    snprintf(path, sizeof(path), "/proc/%s/ns/user", pid_str);

    pid_userns = open(path, O_RDONLY);
    if (pid_userns == -1)
        errExit("open-PID");

    /* If the PID is in the target user namespace, then it has
       whatever capabilities are in its sets. */

    if (ns_equal(pid_userns, target_userns)) {
        printf("PID %s is in the target namespace.\n", pid_str);
        printf("Subject to LSM checks, it has the following capabilities:\n");

        display_process_capabilities(pid);

    } else {

        /* Otherwise, we need to walk through the ancestors of the target
           user namespace to see if PID is in an ancestor user namespace */

        int desc_userns = is_ancestor_userns(pid_userns, target_userns);

        if (desc_userns == -1) {

            /* PID is not in an ancestor user namespace of 'target_userns'. */

            printf("PID %s is not in an ancestor user namespace.\n", pid_str);
            printf("Therefore, it has no capabilities in the target "
                    "namespace.\n");
        } else {

            /* At this point, we found that PID is in a user namespace that
               is an ancestor of the target user namespace, and 'desc_userns'
               refers to the immediate descendant of PID's user namespace in
               the chain of user namespaces from the user namespace of PID to
               the target user namespace. If the effective UID of PID matches
               the owner UID of that descendant user namespace, then PID has
               all capabilities in the descendant namespace(s); otherwise, it
               just has the capabilities that are in its sets. */

            bool is_owner_of_userns = euid_of_process(pid) ==
                                        uid_of_userns_owner(desc_userns);

            printf("PID %s is in an ancestor user namespace", pid_str);

            if (is_owner_of_userns) {
                printf(" and its effective UID matches\n");
            } else {
                printf(", but its effective UID does not match\n");
            }

            printf("the owner of the immediate child user "
                    "namespace of that ancestor namespace.\n");

            if (is_owner_of_userns) {
                printf("Therefore, subject to LSM checks, it has all "
                        "capabilities in the target\n"
                        "namespace!\n");
            } else {
                printf("Therefore, subject to LSM checks, it has only the "
                        "capabilities that are in its\n"
                        "sets, which are:\n");
                display_process_capabilities(pid);
            }

            if (desc_userns != target_userns)   /* Prevent double close() */
                if (close(desc_userns) == -1)
                    errExit("close-desc_userns");
        }
    }

    if (close(target_userns) == -1)
        errExit("close-target_userns");
    if (close(pid_userns) == -1)
        errExit("close-pid_userns");

    exit(EXIT_SUCCESS);
}
