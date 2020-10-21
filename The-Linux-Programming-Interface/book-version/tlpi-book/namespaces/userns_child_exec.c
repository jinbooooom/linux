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

/* userns_child_exec.c

   Create a child process that executes a shell command in new namespace(s).

   See https://lwn.net/Articles/532593/ (note, however, that the program has
   been expanded significantly since then).
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/prctl.h>
#include <limits.h>
#include <linux/securebits.h>
#include <sys/capability.h>
#include <sys/mman.h>
#include "cap_functions.h"
#include "userns_functions.h"

#ifndef CLONE_NEWCGROUP         /* Added in Linux 4.6 */
#define CLONE_NEWCGROUP         0x02000000
#endif

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int pipe_fd[2];  /* Pipe used to synchronize parent and child */

struct option_list {
    char  opt;
    char *val;
};

#define MAX_OPT 100

struct cmd_options {
    char **argv;
    int    flags;
    int    create_root_mappings;
    int    deny_setgroups;
    int    verbose;
    char  *uid_map;
    char  *gid_map;
    int    opt_cnt;
    struct option_list opt_list[MAX_OPT];
};

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] cmd [arg...]\n\n", pname);
    fprintf(stderr, "Create a child process that executes a shell "
            "command in (typically) a new user\n"
            "namespace, and possibly also other new namespaces.\n\n");
    fprintf(stderr, "Options can be:\n\n");
#define fpe(str) fprintf(stderr, "    %s", str);
    fpe("-C          New cgroup namespace\n");
    fpe("-i          New IPC namespace\n");
    fpe("-m          New mount namespace\n");
    fpe("-n          New network namespace\n");
    fpe("-p          New PID namespace\n");
    fpe("-u          New UTS namespace\n");
    fpe("-U          New user namespace\n");
    fpe("-M uid_map  Specify UID map for user namespace\n");
    fpe("-G gid_map  Specify GID map for user namespace\n");
    fpe("-D          Do not write \"deny\" to /proc/PID/setgroups before\n");
    fpe("            updating GID map\n");
    fpe("-r          Create 'root' mappings: map user's UID and GID to "
        "0 in user\n");
    fpe("            namespace (equivalent to: -M '0 <uid> 1' "
        "-G '0 <gid> 1')\n");
    fpe("-v          Display verbose messages\n\n");
    fpe("If -r, -M, or -G is specified, -U is required.\n\n");
    fpe("It is not permitted to specify both -r and either -M or -G.\n");
    fpe("\n");
    fpe("Map strings for -M and -G consist of records of the form:\n");
    fpe("\n");
    fpe("    ID-inside-ns   ID-outside-ns   len\n");
    fpe("\n");
    fpe("A map string can contain multiple records, separated"
        " by commas;\n");
    fpe("the commas are replaced by newlines before writing"
        " to map files.\n");
    fprintf(stderr, "\nThe following additional options (primarily useful "
            "when experimenting with\n"
            "user namespaces) are repeatable: they can appear multiple times "
            "and are\n"
            "performed in the child in the order that they are specified, "
            "just before\n"
            "'cmd' is execed:\n\n");
    fpe("-I          Push all possible capabilities into inheritable set\n")
    fpe("-A          Push all possible capabilities into inheritable and\n")
    fpe("            ambient sets\n");
    fpe("-s <uid>    Set all process UIDs to <uid>\n");
    fpe("-S r,e,s    Set real/effective/saved-set UIDs\n");
    fpe("-b <bits>   Set securebits flags; 'bits' can be '0' to clear "
        "the flags\n");
    fpe("            or one or more of:\n");
    fpe("                'a' - NO_CAP_AMBIENT_RAISE\n");
    fpe("                'A' - NO_CAP_AMBIENT_RAISE_LOCKED\n");
    fpe("                'k' - KEEP_CAPS\n");
    fpe("                'K' - KEEP_CAPS_LOCKED\n");
    fpe("                'r' - NOROOT\n");
    fpe("                'R' - NOROOT_LOCKED\n");
    fpe("                's' - NO_SETUID_FIXUP\n");
    fpe("                'S' - NO_SETUID_FIXUP_LOCKED\n");
    fpe("-d          Display process credentials and capabilities\n");
    fpe("-w <nsecs>  Wait (sleep) for <nsecs> seconds\n");
    fpe("-x <caps>   Set process capabilities; <caps> as per "
            "cap_from_text(3)\n");
    fpe("-X [peiba]{+|-}<cap-name>\n");
    fpe("            Modify one or more process sets by adding or removing\n");
    fpe("            a capability.\n");
    fpe("            Each set is modified as an individual operation\n");
    fpe("            in the order specified before +/-.\n");
    exit(EXIT_FAILURE);
}

/* Raise as many inheritable and (optionally) ambient capabilities as
   possible. The attempt is "best effort": we ignore any errors that
   occur because a capability can't be raised. */

static void
raiseInheritableAndAmbientCaps(bool doAmbient)
{
    /* Walk through the range of possible capabilities trying to add each
       capability to the inheritable and ambient sets. In order to raise
       an ambient capability, it must first be raised in the process
       inheritable set. */

    for (int cap = 0; cap <= CAP_LAST_CAP; cap++) {
        modifyCapSetting(CAP_INHERITABLE, cap, CAP_SET);
        if (doAmbient)
            prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0);
    }
}

/* Raise or lower a single capability in multiple process capability sets.
   The argument is a string of the form:

        <sets><op><cap-name>

   where:
        <sets> is one of more of the letters [peiba], denoting the various
                process capability sets (permitted, effective, inheritable,
                bounding, ambient).
        <op> is either '+', meaning raise the capability or '-', meaning
                lower the capability.
        <cap-name> is the name of the capability to be modified in the sets.

   The modifications to the capability sets are performed as individual
   operations, in the order specified in <sets>.
*/

static void
modify_individual_capability(char *optval)
{
    cap_value_t cap;
    char *op;
    int flag;
    int capval;
    bool bad_cap;

    bad_cap = false;
    for (op = optval; *op; op++) {
        if (*op == '+' || *op == '-')
            break;
        if (strchr("peiba", *op) == NULL)
            bad_cap = true;
    }

    /* If we got a bad capability character or there are not least two more
       characters (<op> and another character), diagnose an error */

    if (*op == '\0' || *(op + 1) == '\0' || bad_cap) {
        fprintf(stderr, "Badly formed -X option\n");
        exit(EXIT_FAILURE);
    }

    /* After <op>, there should be a valid capability name */

    if (cap_from_name(op + 1, &cap) == -1) {
        fprintf(stderr, "-X: bad capability name\n");
        exit(EXIT_FAILURE);
    }

    /* Walk through each of the letters--which specify capability sets--at the
       start of the string, modifying the specified capability setting */

    for (char *p = optval; p != op; p++) {
        switch (*p) {
        case 'p':
        case 'i':
        case 'e':
            flag = (*p == 'p') ? CAP_PERMITTED :
                   (*p == 'e') ? CAP_EFFECTIVE : CAP_INHERITABLE;
            capval = (*op == '+') ? CAP_SET : CAP_CLEAR;

            if (modifyCapSetting(flag, cap, capval) == -1) {
                fprintf(stderr, "-X: modifyCapSetting() failed while "
                        "%s '%s' in '%c'\n",
                        (capval == CAP_SET) ? "raising" : "lowering",
                        op + 1, *p);
                exit(EXIT_FAILURE);
            }
            break;

        case 'b':
            if (*op == '+') {
                fprintf(stderr, "Can't add capabilities to the bounding set\n");
                exit(EXIT_FAILURE);
            }
            if (prctl(PR_CAPBSET_DROP, cap, 0, 0, 0) == -1)
                errExit("-X: PR_CAPBSET_DROP");
            break;

        case 'a':
            if (prctl(PR_CAP_AMBIENT,
                        (*op == '+') ? PR_CAP_AMBIENT_RAISE :
                                      PR_CAP_AMBIENT_LOWER,
                        cap, 0, 0) == -1)
                errExit("-X: PR_CAP_AMBIENT");
            break;

        default:
            fprintf(stderr, "Bad switch case ('-X'): %c\n", *p);
            exit(EXIT_FAILURE);
        }
    }
}

/* Set process securebits */

static void
set_securebits(char *optval)
{
    int secbits;
    char *p;

    secbits = 0;
    for (p = optval; *p != '\0'; p++) {
        switch (*p) {
        case 'a': secbits |= SECBIT_NO_CAP_AMBIENT_RAISE;               break;
        case 'A': secbits |= SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED;        break;
        case 'k': secbits |= SECBIT_KEEP_CAPS;                          break;
        case 'K': secbits |= SECBIT_KEEP_CAPS_LOCKED;                   break;
        case 'r': secbits |= SECBIT_NOROOT;                             break;
        case 'R': secbits |= SECBIT_NOROOT_LOCKED;                      break;
        case 's': secbits |= SECBIT_NO_SETUID_FIXUP;                    break;
        case 'S': secbits |= SECBIT_NO_SETUID_FIXUP_LOCKED;             break;
        case '0': secbits = 0;                                          break;
        default:
            fprintf(stderr, "Unexpected value for '-b' "
                    "(securebits flag): %c\n", *p);
            exit(EXIT_FAILURE);
        }
    }

    if (prctl(PR_SET_SECUREBITS, secbits) == -1)
        errExit("prctl-PR_SET_SECUREBITS");
}

/* Set process UIDs individually, according to three comma-separated
   values in 'optval' */

static void
set_process_uids(char *optval)
{
    char *comma1, *comma2;

    comma2 = NULL;
    comma1 = strchr(optval, ',');
    if (comma1 != NULL)
        comma2 = strchr(comma1 + 1, ',');
    if (comma2 == NULL) {
        fprintf(stderr, "Expected three comma-separated UIDs: -S %s\n",optval);
        exit(EXIT_FAILURE);
    }
    *comma1 = '\0';
    *comma2 = '\0';
    if (setresuid(atoi(optval), atoi(comma1 + 1), atoi(comma2 + 1)) == -1)
        errExit("-S failed (setresuid)");
}

/* Perform the actions requested in the repeatable options that were
   specified in command-line options. */

static void
perform_repeatable_options(struct cmd_options *opts)
{
    int display_cnt = 0;
    uid_t newuid;
    cap_t caps;
    char buf[10];
    int j;

    for (j = 0; j < opts->opt_cnt; j++) {
        switch (opts->opt_list[j].opt) {

        case 'a': /* Raise as many ambient capabilities as possible */
            raiseInheritableAndAmbientCaps(true);
            break;

        case 'b': /* Set process securebits */
            set_securebits(opts->opt_list[j].val);
            break;

        case 'd': /* Display credentials and capabilities */
            display_cnt++;
            snprintf(buf, sizeof(buf), "[-d %d] ", display_cnt);
            display_creds_and_caps(buf);
            break;

        case 'h': /* Raise as many inheritable capabilities as possible */
            raiseInheritableAndAmbientCaps(false);
            break;

        case 's': /* Set real/effective/saved-set UIDs to a single value */
            newuid = atoi(opts->opt_list[j].val);
            if (setresuid(newuid, newuid, newuid) == -1)
                errExit("setresuid");
            break;

        case 'S': /* Set real/effective/saved-set UIDs individually */
            set_process_uids(opts->opt_list[j].val);
            break;

        case 'w': /* Sleep for a bit */
            sleep(atoi(opts->opt_list[j].val));
            break;

        case 'x': /* Set process capabilities as specified by
                     cap_from_text(3)-style string */
            caps = cap_from_text(opts->opt_list[j].val);
            if (caps == NULL)
                errExit("-x: cap_from_text");
            if (cap_set_proc(caps) == -1)
                errExit("-x: cap_set_proc()");
            cap_free(caps);
            break;

        case 'X': /* Modify a single capability in process sets */
            modify_individual_capability(opts->opt_list[j].val);
            break;

        default:
            fprintf(stderr, "Unexpected option (-%c) in "
                    "perform_repeatable_options()\n", opts->opt_list[j].opt);
        }
    }
}

/* Start function for cloned child */

static int
childFunc(void *arg)
{
    struct cmd_options *opts = (struct cmd_options *) arg;
    char ch;

    /* Wait until the parent has updated the UID and GID mappings.
       See the comment in main(). We wait for end of file on a
       pipe that will be closed by the parent process once it has
       updated the mappings. */

    close(pipe_fd[1]);          /* Close our descriptor for the write
                                   end of the pipe so that we see EOF
                                   when parent closes its descriptor */
    if (read(pipe_fd[0], &ch, 1) != 0) {
        fprintf(stderr,
                "Failure in child: read from pipe returned != 0\n");
        exit(EXIT_FAILURE);
    }

    close(pipe_fd[0]);  /* We no longer need the pipe */

    perform_repeatable_options(opts);

    /* Execute a shell command */

    if (opts->verbose)
        printf("About to exec: %s\n", opts->argv[0]);

    execvp(opts->argv[0], opts->argv);
    errExit("execvp");
}

/* Use the information in 'opts' to update the UID and GID map of 'child_pid' */

static void
update_child_maps(struct cmd_options *opts, pid_t child_pid)
{
    const int MAP_BUF_SIZE = 100;
    char map_buf[MAP_BUF_SIZE];
    char map_path[PATH_MAX];

    /* UID map */

    if (opts->uid_map != NULL || opts->create_root_mappings) {
        snprintf(map_path, PATH_MAX, "/proc/%ld/uid_map", (long) child_pid);
        if (opts->create_root_mappings) {
            snprintf(map_buf, MAP_BUF_SIZE, "0 %ld 1", (long) getuid());
            opts->uid_map = map_buf;
        }
        if (update_map(opts->uid_map, map_path) == -1)
            errExit("update_map: uid_map");
    }

    /* GID map */

    if (opts->gid_map != NULL || opts->create_root_mappings) {
        if (opts->deny_setgroups) {
            if (proc_setgroups_write(child_pid, "deny") == -1)
                errExit("proc_setgroups_write");
        }

        snprintf(map_path, PATH_MAX, "/proc/%ld/gid_map",
                (long) child_pid);
        if (opts->create_root_mappings) {
            snprintf(map_buf, MAP_BUF_SIZE, "0 %ld 1", (long) getgid());
            opts->gid_map = map_buf;
        }
        if (update_map(opts->gid_map, map_path) == -1)
            errExit("update_map: gid_map");
    }
}

static void
parse_command_options(int argc, char *argv[], struct cmd_options *opts)
{
    int opt;

    /* Parse command-line options. The initial '+' character in the final
       getopt(3) argument prevents GNU-style permutation of command-line
       options. Preventing that is useful, since sometimes the 'command' to
       be executed by this program itself has command-line options. We
       don't want getopt() to treat those as options to this program. */

    opts->flags = 0;
    opts->verbose = 0;
    opts->opt_cnt = 0;
    opts->gid_map = NULL;
    opts->uid_map = NULL;
    opts->create_root_mappings = 0;
    opts->deny_setgroups = 1;
    while ((opt = getopt(argc, argv,
                         "+CimnuUM:G:rDvpAIs:b:S:dx:X:w:")) != -1) {
        switch (opt) {
        case 'C': opts->flags |= CLONE_NEWCGROUP;       break;
        case 'i': opts->flags |= CLONE_NEWIPC;          break;
        case 'm': opts->flags |= CLONE_NEWNS;           break;
        case 'n': opts->flags |= CLONE_NEWNET;          break;
        case 'p': opts->flags |= CLONE_NEWPID;          break;
        case 'u': opts->flags |= CLONE_NEWUTS;          break;
        case 'U': opts->flags |= CLONE_NEWUSER;         break;
        case 'M': opts->uid_map = optarg;               break;
        case 'G': opts->gid_map = optarg;               break;
        case 'r': opts->create_root_mappings = 1;       break;
        case 'D': opts->deny_setgroups = 0;             break;
        case 'v': opts->verbose = 1;                    break;

        /* Repeatable options: */

        case 'I':
        case 'A':
        case 'b':
        case 's':
        case 'd':
        case 'x':
        case 'X':
        case 'w':
            if (opts->opt_cnt >= MAX_OPT) {
                fprintf(stderr, "Too many repeatable options (maximum: %d)\n",
                        MAX_OPT);
                exit(EXIT_FAILURE);
            }

            opts->opt_list[opts->opt_cnt].opt = opt;
            opts->opt_list[opts->opt_cnt].val = NULL;
            if (opt == 's' || opt == 'S' || opt == 'x' || opt == 'X' ||
                    opt == 'w' || opt == 'b')
                opts->opt_list[opts->opt_cnt].val = optarg;
            opts->opt_cnt++;
            break;

        default:
            usage(argv[0]);
        }
    }

    /* -M, -G, or -r without -U is nonsensical */

    if (((opts->uid_map != NULL || opts->gid_map != NULL ||
            opts->create_root_mappings) && !(opts->flags & CLONE_NEWUSER)) ||
            (opts->create_root_mappings &&
                (opts->uid_map != NULL || opts->gid_map != NULL)))
        usage(argv[0]);

    if (optind >= argc)
        usage(argv[0]);

    opts->argv = &argv[optind];
}

#define STACK_SIZE (1024 * 1024)

int
main(int argc, char *argv[])
{
    pid_t child_pid;
    char *stack;
    struct cmd_options opts;

    parse_command_options(argc, argv, &opts);

    /* If this binary has been set up with capabilities (e.g., CAP_SETUID,
       so that arbitrary UID maps may be written), then the ownership of the
       /proc/PID files of this process and the child that we are about to
       create will be made root:root, for reasons described in prctl(2) (see
       the discussion of the "dumpable" flag) and proc(5) (see the discussion
       of the /proc/[pid] directory). Because of this change to the ownership
       of the /proc/PID files, the parent process would need the
       CAP_DAC_OVERRIDE capability to be able to update the child's UID and GID
       map files. Rather than require this capability, we reset the process
       "dumpable" flag to 1, which causes the ownership of the files in the
       /proc/PID directory to revert to the process's effective UID and GID. */

    if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1)
        errExit("prctl-PR_SET_DUMPABLE");

    /* We use a pipe to synchronize the parent and child, in order to
       ensure that the parent sets the UID and GID maps before the child
       calls execve(). This ensures that the child maintains its
       capabilities during the execve() in the common case where we
       want to map the child's effective user ID to 0 in the new user
       namespace. Without this synchronization, the child would lose
       its capabilities if it performed an execve() with nonzero
       user IDs (see the capabilities(7) man page for details of the
       transformation of a process's capabilities during execve()). */

    if (pipe(pipe_fd) == -1)
        errExit("pipe");

    /* Create the child in new namespace(s) */

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");

    child_pid = clone(childFunc, stack + STACK_SIZE,
                      opts.flags | SIGCHLD, &opts);
    if (child_pid == -1)
        errExit("clone");

    /* Parent falls through to here */

    if (opts.verbose)
        printf("%s: PID of child created by clone() is %ld\n",
                argv[0], (long) child_pid);

    /* Update the UID and GID maps in the child */

    update_child_maps(&opts, child_pid);

    /* Close the write end of the pipe, to signal to the child that we
       have updated the UID and GID maps */

    close(pipe_fd[1]);

    if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    if (opts.verbose)
        printf("%s: terminating\n", argv[0]);

    exit(EXIT_SUCCESS);
}
