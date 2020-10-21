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

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/audit.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "scm_functions.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static void
sigchldHandler(int sig)
{
    char *msg  = "\tS: target has terminated; bye\n";

    write(STDOUT_FILENO, msg, strlen(msg));
    _exit(EXIT_SUCCESS);
}

static int
seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(__NR_seccomp, operation, flags, args);
}

/* Values from command-line options */

struct cmdLineOpts {
    int  delaySecs;     /* Delay time for responding to notifications */
    int  secondFilter;  /* Install a second BPF filter? */
};

/* The following is the x86-64-specific BPF boilerplate code for checking that
   the BPF program is running on the right architecture + ABI. At completion
   of these instructions, the accumulator contains the system call number. */

/* For the x32 ABI, all system call numbers have bit 30 set */

#define X32_SYSCALL_BIT         0x40000000

#define X86_64_CHECK_ARCH_AND_LOAD_SYSCALL_NR \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                (offsetof(struct seccomp_data, arch))), \
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 0, 2), \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                 (offsetof(struct seccomp_data, nr))), \
        BPF_JUMP(BPF_JMP | BPF_JGE | BPF_K, X32_SYSCALL_BIT, 0, 1), \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS)

/* installNotifyFilter() installs a seccomp filter that generates
   user-space notifications (SECCOMP_RET_USER_NOTIF) when the process
   calls mkdir(2); the filter allows all other system calls.

   The function return value is a file descriptor from which the
   user-space notifications can be fetched. */

static int
installNotifyFilter(void)
{
    struct sock_filter filter[] = {
        X86_64_CHECK_ARCH_AND_LOAD_SYSCALL_NR,

        /* mkdir() triggers notification to user-space supervisor */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mkdir, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),

        /* Every other system call is allowed */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    /* Install the filter with the SECCOMP_FILTER_FLAG_NEW_LISTENER flag;
       as a result, seccomp() returns a notification file descriptor. */

    /* Only one listening file descriptor can be established. An attempt to
       establish a second listener yields an EBUSY error. */

    /* Because of conflicting uses of the system call return value,
       'flags' can't contain both SECCOMP_FILTER_FLAG_NEW_LISTENER
       and SECCOMP_FILTER_FLAG_TSYNC (or an EINVAL error results). */

    int notifyFd = seccomp(SECCOMP_SET_MODE_FILTER,
                           SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (notifyFd == -1)
        errExit("seccomp-install-notify-filter");

    return notifyFd;
}

/* Use poll() to examine a file descriptor and display the results */

static void
pollit(int fd)
{
    struct pollfd pollfd;

    pollfd.events = POLLIN | POLLOUT;
    pollfd.fd = fd;
    int ready = poll(&pollfd, 1, 0);
    if (ready == -1)
        errExit("poll");

    printf("revents: %#x; %s%s\n", pollfd.revents,
            (pollfd.revents & POLLIN) ? "POLLIN " : "",
            (pollfd.revents & POLLOUT) ? "POLLOUT " : "");
}

/* installFilter2() optionally installs a second BPF filter in order to allow
   experiments with the precedence of SECCOMP_RET_USER_NOTIF relative to other
   filter return values. As with the other filter, this filter performs
   special treatment of mkdir(2) and allows all other system calls. */

static void
installFilter2(struct cmdLineOpts *opts)
{
    struct sock_filter filter[] = {
        X86_64_CHECK_ARCH_AND_LOAD_SYSCALL_NR,

        /* Treat mkdir() system calls specially */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mkdir, 1, 0),

        /* Every other system call is allowed */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        /* The last entry in the BPF program will be replaced by a "return"
           instruction; see below */

        { 0, 0, 0, 0 },
    };

    struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    /* Depending on the value of the "-f" command-line option, place either
       a SECCOMP_RET_ERRNO instruction in the BPF program, or otherwise a
       SECCOMP_RET_TRACE instruction. This can be used to illustrate that
       SECCOMP_RET_ERRNO has higher precedence than the SECCOMP_RET_USER_NOTIF
       returned by the other filter, with the result that the user-space
       notification will not occur. By contrast, SECCOMP_RET_TRACE has lower
       precedence (so that the user-space notification does occur). */

    const struct sock_filter retTrace = BPF_STMT(BPF_RET + BPF_K,
                                              SECCOMP_RET_TRACE);
    const struct sock_filter retErrno = BPF_STMT(BPF_RET + BPF_K,
                                              SECCOMP_RET_ERRNO | ENOTSUP);

    filter[prog.len - 1] = (opts->secondFilter == SECCOMP_RET_ERRNO) ?
                                        retErrno : retTrace;

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &prog) == -1)
        errExit("seccomp-install-filter-2");
}

/* Handler for the SIGINT signal in the target process */

static void
handler(int sig)
{
    /* UNSAFE: This handler uses non-async-signal-safe functions
       (printf(); see TLPI Section 21.1.2) */

    printf("T: received signal\n");
}

/* Close a pair of sockets created by socketpair() */

static void
closeSocketPair(int sockPair[2])
{
    if (close(sockPair[0]) == -1)
        errExit("closeSocketPair-close-0");
    if (close(sockPair[1]) == -1)
        errExit("closeSocketPair-close-1");
}

/* Implementation of the target process; create a child process that:

   (1) installs a seccomp filter with the
       SECCOMP_FILTER_FLAG_NEW_LISTENER flag;
   (2) writes the seccomp notification file descriptor returned from
       the previous step onto the UNIX domain socket, 'sockPair[0]';
   (3) calls mkdir(2) for each element of 'argv'.

   The function return value in the parent is the PID of the child
   process; the child does not return from this function. */

static pid_t
targetProcess(int sockPair[2], char *argv[], struct cmdLineOpts *opts)
{
    pid_t targetPid = fork();
    if (targetPid == -1)
        errExit("fork");

    if (targetPid > 0)          /* In parent, return PID of child */
        return targetPid;

    /* Child falls through to here */

    printf("T: PID = %ld\n", (long) getpid());

    /* Install a handler for the SIGINT signal */

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    /* Install seccomp filter(s) */

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
        errExit("prctl");

    int notifyFd = installNotifyFilter();

    if (opts->secondFilter != -1)
        installFilter2(opts);

    /* Pass the notification file descriptor to the tracing process over
       a UNIX domain socket */

    if (sendfd(sockPair[0], notifyFd) == -1)
        errExit("sendfd");

    /* Notification and socket FDs are no longer needed in target process */

    if (close(notifyFd) == -1)
        errExit("close-target-notify-fd");

    closeSocketPair(sockPair);

    /* Perform a mkdir() call for each of the command-line arguments */

    for (char **ap = argv; *ap != NULL; ap++) {
        printf("\nT: about to mkdir(\"%s\")\n", *ap);

        int s = mkdir(*ap, 0700);
        if (s == -1)
            perror("T: ERROR: mkdir(2)");
        else
            printf("T: SUCCESS: mkdir(2) returned %d\n", s);
    }

    printf("\nT: terminating\n");
    exit(EXIT_SUCCESS);
}

/* Check that the notification ID provided by a SECCOMP_IOCTL_NOTIF_RECV
   operation is still valid. It will no longer be valid if the process
   has terminated. This operation can be used when accessing /proc/PID
   files in the target process in order to avoid TOCTOU race conditions
   where the PID that is returned by SECCOMP_IOCTL_NOTIF_RECV terminates
   and is reused by another process. */

static void
checkNotificationIdIsValid(int notifyFd, uint64_t id, char *tag)
{
    if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_ID_VALID, &id) == -1) {
        fprintf(stderr, "\tS: notification ID check (%s): "
                "target has terminated!!!\n", tag);

        exit(EXIT_FAILURE);
    }
}

/* Access the memory of the target process in order to discover the
   pathname that was given to mkdir() */

static void
getTargetPathname(struct seccomp_notif *req, int notifyFd,
                  char *path, size_t len)
{
    char procMemPath[PATH_MAX];

    snprintf(procMemPath, sizeof(procMemPath), "/proc/%d/mem", req->pid);

    int procMemFd = open(procMemPath, O_RDONLY | O_CLOEXEC);
    if (procMemFd == -1)
        errExit("Supervisor: open");

    /* Check that the process whose info we are accessing is still alive */

    checkNotificationIdIsValid(notifyFd, req->id, "post-open");

    /* Since the SECCOMP_IOCTL_NOTIF_ID_VALID operation (performed
       in checkNotificationIdIsValid()) succeeded, we know that the
       /proc/PID/mem file descriptor that we opened corresponded to the
       process for which we received a notification. If that process
       subsequently terminates, then read() on that file descriptor
       will return 0 (EOF). This can be tested by (1) uncommenting
       the sleep() call below (and rebuilding the program); and
       (2) killing the target process during the sleep(). */

    // printf("About to sleep in target\n");
    // sleep(15);

    /* Read bytes at the location containing the pathname argument
       (i.e., the first argument) of the mkdir(2) call */

    ssize_t s = pread(procMemFd, path, sizeof(path), req->data.args[0]);
    if (s == -1)
        errExit("Supervisor: pread");

    if (s == 0) {
        fprintf(stderr, "\tS: pread() of /proc/PID/mem "
                "returned 0 (EOF)\n");
        exit(EXIT_FAILURE);
    }

    if (close(procMemFd) == -1)
        errExit("Supervisor: close-/proc/PID/mem");
}

/* Handle notifications that arrive via the SECCOMP_RET_USER_NOTIF file
   descriptor, 'notifyFd'. */

static void
handleNotifications(int notifyFd, struct cmdLineOpts *opts)
{
    struct seccomp_notif_sizes sizes;
    char path[PATH_MAX];    /* For simplicity, we assume that the
                               pathname given to mkdir() is no more than
                               PATH_MAX bytes; but this might not be true. */

    /* Discover the sizes of the structures that are used to receive
       notifications and send notification responses, and allocate
       buffers of those sizes. */

    if (seccomp(SECCOMP_GET_NOTIF_SIZES, 0, &sizes) == -1)
        errExit("Supervisor: seccomp-SECCOMP_GET_NOTIF_SIZES");

    struct seccomp_notif *req = malloc(sizes.seccomp_notif);
    if (req == NULL)
        errExit("Supervisor: malloc");

    struct seccomp_notif_resp *resp = malloc(sizes.seccomp_notif_resp);
    if (resp == NULL)
        errExit("Supervisor: malloc");

    /* Loop handling notifications */

    for (;;) {
        /* Wait for next notification, returning info in '*req' */

        memset(req, 0, sizes.seccomp_notif);    /* Required since Linux 5.5 */
        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            if (errno == EINTR)
                    continue;

            errExit("Supervisor: ioctl-SECCOMP_IOCTL_NOTIF_RECV");
        }

        printf("\tS: got notification (ID %#llx) for PID %d\n",
                req->id, req->pid);

        /* The only system call that can generate a notification event
           is mkdir(2). Nevertheless, we check that the notified system
           call is indeed mkdir() as kind of future-proofing of this
           code in case the seccomp filter is later modified to
           generate notifications for other system calls. */

        if (req->data.nr != __NR_mkdir) {
            printf("\tS: notification contained unexpected "
                    "system call number; bye!!!\n");
            exit(EXIT_FAILURE);
        }

        /* If a delay interval was specified on the command line, then
           delay for the specified number of seconds. This can be used
           to demonstrate the following:

           (1) The target process is blocked until the supervisor sends
               a response.
           (2) If the blocked system call is interrupted by a signal
               handler, then the SECCOMP_IOCTL_NOTIF_SEND operation
               fails with the error ENOENT.
           (3) If the target process terminates, then we can discover
               this using the SECCOMP_IOCTL_NOTIF_ID_VALID operation
               (which is employed by checkNotificationIdIsValid()). */

        if (opts->delaySecs > 0) {
            printf("\tS: delaying for %d seconds:", opts->delaySecs);
            checkNotificationIdIsValid(notifyFd, req->id, "pre-delay");

            for (int d = opts->delaySecs; d > 0; d--) {
                printf(" %d", d);
                sleep(1);
            }
            printf("\n");

            checkNotificationIdIsValid(notifyFd, req->id, "post-delay");
        }

        getTargetPathname(req, notifyFd, path, sizeof(path));

        /* Prepopulate some fields of the response */

        resp->id = req->id;     /* Response includes notification ID */
        resp->flags = 0;
        resp->val = 0;

        /* If the directory is in /tmp, then create it on behalf of
           the supervisor; if the pathname starts with '.', tell the
           kernel to let the target process execute the mkdir();
           otherwise, give an error for a directory pathname in
           any other location. */

        if (strncmp(path, "/tmp/", strlen("/tmp/")) == 0) {
            printf("\tS: executing: mkdir(\"%s\", %#llo)\n",
                    path, req->data.args[1]);

            if (mkdir(path, req->data.args[1]) == 0) {
                resp->error = 0;            /* "Success" */
                resp->val = strlen(path);   /* Used as return value of
                                               mkdir() in target process */
                printf("\tS: success! spoofed return = %lld\n",
                        resp->val);
            } else {

                /* If mkdir() failed in the supervisor, pass the error
                   back to the target */

                resp->error = -errno;
                printf("\tS: failure! (errno = %d; %s)\n", errno,
                        strerror(errno));
            }
        } else if (strncmp(path, "./", strlen("./")) == 0) {
            resp->error = resp->val = 0;
            resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
            printf("\tS: target can execute system call\n");
        } else {
            resp->error = -EOPNOTSUPP;
            printf("\tS: spoofing error response (%s)\n",
                    strerror(-resp->error));
        }

        /* Send a response to the notification */

        printf("\tS: sending response "
                "(flags = %#x; val = %lld; error = %d)\n",
                resp->flags, resp->val, resp->error);

        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            if (errno == ENOENT)
                printf("\tS: response failed with ENOENT; "
                        "perhaps target process's syscall was "
                        "interrupted by a signal?\n");
            else
                perror("ioctl-SECCOMP_IOCTL_NOTIF_SEND");
        }

        /* If the pathname is just "/bye", then the supervisor
           terminates. This allows us to see what happens if the
           target process makes further calls to mkdir(2). */

        if (strcmp(path, "/bye") == 0) {
            printf("\tS: terminating **********\n");
            exit(EXIT_FAILURE);
        }
    }
}

/* Implementation of the supervisor process:

   (1) obtains the seccomp notification file descriptor from 'sockPair[1]';
   (2) handles notifications that arrive on that file descriptor. */

static void
supervisor(int sockPair[2], struct cmdLineOpts *opts)
{
    int notifyFd = recvfd(sockPair[1]);
    if (notifyFd == -1)
        errExit("Supervisor: recvfd");

    closeSocketPair(sockPair);  /* We no longer need the socket pair */

    handleNotifications(notifyFd, opts);
}

/* Diagnose an error in command-line option or argument usage */

static void
usageError(char *msg, char *pname)
{
    if (msg != NULL)
        fprintf(stderr, "%s\n", msg);

#define fpe(msg) fprintf(stderr, "      " msg);
    fprintf(stderr, "Usage: %s [options] <dir> <dir>...\n", pname);
    fpe("Options\n");
    fpe("-d <nsecs>    Supervisor delays 'nsecs' before inspecting target\n");
    fpe("-f <val>      Install second filter whose return value is:\n");
    fpe("              'e' - SECCOMP_RET_ERRNO\n");
    fpe("              't' - SECCOMP_RET_TRACE\n");
    exit(EXIT_FAILURE);
}

/* Parse command-line options, returning option info in 'opts' */

static void
parseCommandLineOptions(int argc, char *argv[], struct cmdLineOpts *opts)
{
    int opt;

    opts->secondFilter = -1;
    opts->delaySecs = 0;

    while ((opt = getopt(argc, argv, "d:f:")) != -1) {
        switch (opt) {
        case 'f':       /* Install a second BPF filter */
            if (optarg[0] == 'e')
                opts->secondFilter = SECCOMP_RET_ERRNO;
            else if (optarg[0] == 't')
                opts->secondFilter = SECCOMP_RET_TRACE;
            else
                usageError("Bad value for -f", argv[0]);
            break;

        case 'd':       /* Delay time before sending notification response */
            opts->delaySecs = atoi(optarg);
            break;

        default:
            usageError("Bad option", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /* There should be at least one argument after the options */

    if (optind >= argc)
        usageError("At least one pathname argument should be supplied",
                argv[0]);
}

int
main(int argc, char *argv[])
{
    int sockPair[2];
    struct cmdLineOpts opts;

    setbuf(stdout, NULL);

    parseCommandLineOptions(argc, argv, &opts);

    /* Create a UNIX domain socket that is used to pass the seccomp
       notification file descriptor from the target process to the
       supervisor process. */

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockPair) == -1)
        errExit("socketpair");

    /* Create a child process--the "target"--that installs seccomp
       filtering. The target process writes the seccomp notification
       file descriptor onto 'sockPair[0]' and then calls mkdir(2) for
       each directory in the command-line arguments. */

    (void) targetProcess(sockPair, &argv[optind], &opts);

    /* Catch SIGCHLD when target terminates, so that supervisor knows
       that there is nothing more to do. */

    struct sigaction sa;
    sa.sa_handler = sigchldHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");

    supervisor(sockPair, &opts);

    exit(EXIT_SUCCESS);
}
