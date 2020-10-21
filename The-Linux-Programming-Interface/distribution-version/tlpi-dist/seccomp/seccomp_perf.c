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

/* seccomp_perf.c

   How much does a simple seccomp() filter cost vis-a-vis a trivial system
   call such as getppid()?

   To test with the in-kernel JIT compiler enabled:

        $ sudo sh -c "echo 1 > /proc/sys/net/core/bpf_jit_enable"
*/
#define _GNU_SOURCE
#include <stddef.h>
#include <fcntl.h>
#include <linux/audit.h>
#include <sys/syscall.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

/* For the x32 ABI, all system call numbers have bit 30 set */

#define X32_SYSCALL_BIT         0x40000000

/* The following is a hack to allow for systems (pre-Linux 4.14) that don't
   provide SECCOMP_RET_KILL_PROCESS, which kills (all threads in) a process.
   On those systems, define SECCOMP_RET_KILL_PROCESS as SECCOMP_RET_KILL
   (which simply kills the calling thread). */

#ifndef SECCOMP_RET_KILL_PROCESS
#define SECCOMP_RET_KILL_PROCESS SECCOMP_RET_KILL
#endif

static int
seccomp(unsigned int operation, unsigned int flags, void *arg)
{
    return syscall(__NR_seccomp, operation, flags, arg);
    // Or: return prctl(PR_SET_SECCOMP, operation, arg);
}

static void
install_filter(void)
{
    struct sock_filter filter[] = {
        /* Load architecture */

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                (offsetof(struct seccomp_data, arch))),

        /* Kill the process if the architecture is not what we expect */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 0, 2),

        /* Load system call number */

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 (offsetof(struct seccomp_data, nr))),

        /* Kill the process if this is an x32 system call (bit 30 is set) */

        BPF_JUMP(BPF_JMP | BPF_JGE | BPF_K, X32_SYSCALL_BIT, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),

        /* Allow system calls other than open() */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_open, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        /* Kill process on open() */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS)
    };

    struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &prog) == -1)
        errExit("seccomp");
}

int
main(int argc, char *argv[])
{
    int nloops;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num-loops> [x]\n", argv[0]);
        fprintf(stderr, "       (use 'x' to run with BPF filter applied)\n");
        exit(EXIT_FAILURE);
    }

    if (argc > 2) {
        printf("Applying BPF filter\n");

        if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
            errExit("prctl");

        install_filter();
    }

    nloops = atoi(argv[1]);

    for (int j = 0; j < nloops; j++)
        getppid();

    exit(EXIT_SUCCESS);
}
