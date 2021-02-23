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

/* seccomp_multiarch.c

   This program demonstrates one of the reasons that a seccomp filter must
   verify the architecture on *each* system call. The point is that some
   hardware platforms support multiple architectures. For example, modern
   x86 hardware supports x86-64, i386, and x32 architectures. The
   architecture is determined by the binary that is executed, and since a
   process may execute multiple binaries over its lifetime (via successive
   calls to execve()), this means that the architecture may change during
   the life of the process. However, once installed, seccomp filters
   persist for the lifetime of the process; in particular, filters persist
   across calls to execve() (if the filters permit calls to execve()), and
   thus across architecture changes.  Thus, a seccomp filter needs to check
   its architecture assumption each time the filter is invoked.

   The idea behind this program is to build binaries for different
   architectures from this source file. A chain of execve() calls inside
   one process can then be triggered by running one of the binaries with
   the other binary/binaries as its command-line argument(s). The first
   binary that is executed installs a seccomp() filter which remains
   installed for the life of the process. This filter causes calls to
   mq_notify(2) to fail, but checks for each of the syscall numbers for
   x86-64, i386, and x32, and fails each different architecture-specific
   system call with a different error. An environment variable is employed
   to ensure that only the first binary installs a seccomp() filter.

   Each binary prints out some status information, calls mq_notify() (and
   prints the resulting error number returned in 'errno'), and execs the
   next binary with the remaining command-line arguments.

   Demonstration steps:

   $ cc      -o seccomp_multiarch      seccomp_multiarch.c -lrt
   $ cc -m32 -o seccomp_multiarch_i386 seccomp_multiarch.c -lrt
   $ ./seccomp_multiarch ./seccomp_multiarch_i386 ./seccomp_multiarch

   Before building 'seccomp_multiarch_i386', you may need to install the
   glibc i386 (32-bit) library (e.g., on Ubuntu, this is libc-dev-i386;
   on Fedora, it is glibc-devel.i686).

   NOTE: This program is designed to be run on an Intel x86-64 system.
   If you run this program on a non-Intel multi-arch platform, the
   definitions of architecture and system call numbers below will
   need to be adjusted.

   NOTE: While it is possible to write seccomp filters that deal with
   multiple architectures, this is not recommended best practice. Rather,
   filters should be written specific to an architecture, and terminate
   the process if the architecture is other than what is expected. Where
   needed, it is best to have one filter per architecture. Trying to create
   a filter that deals with multiple architectures has a number of problems:
   such filters will be larger (and one may easily hit the 4096-instruction
   limit) and slower (because of the added architecture checks).
*/
#define _GNU_SOURCE
#include <mqueue.h>
#include <stddef.h>
#include <fcntl.h>
#include <linux/audit.h>
#include <sys/syscall.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

/* The following is a hack to allow for systems (pre-Linux 4.14) that don't
   provide SECCOMP_RET_KILL_PROCESS, which kills (all threads in) a process.
   On those systems, define SECCOMP_RET_KILL_PROCESS as SECCOMP_RET_KILL
   (which simply kills the calling thread). */

#ifndef SECCOMP_RET_KILL_PROCESS
#define SECCOMP_RET_KILL_PROCESS SECCOMP_RET_KILL
#endif

static int
seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(__NR_seccomp, operation, flags, args);
}

/* Define names for the various system call numbers for the mq_notify()
   system call on Intel architectures:

   x86-64: [cc ...]
        This is the modern 64-bit Intel architecture with 64-bit
        instruction set and 64-bit pointers.

   i386: [cc -m32 ...]
        This is the historical Intel architecture with 32-bit instructions
        and 32-bit pointers (and thus 32-bit address space). As a backward
        compatibility feature, modern 64-bit Intel CPUs can also execute
        i386 programs.

   x32: [cc -mx32 ...]
        This is an alternate ABI for the Intel 64-bit architecture, available
        via the kernel configuration option CONFIG_X86_X32.  This ABI, which
        was added in Linux 3.4 (2012), allows an application to take advantage
        of the modern x86-64 instruction set and 64-bit registers (which are
        more numerous than the register set on i386) while at the same time
        employing 32-bit pointers (and thus 32-bit address space) and 32-bit
        'long'.  The rationale for adding the ABI was that the smaller types
        can result in significant memory savings in some code, and can also
        provide performance improvements in some cases.

        Because some types (e.g., pointers and 'long') under this ABI have
        sizes that are different from the x86-64 ABI, some system calls
        require signatures that differ from x86-64, and thus have different
        syscall numbers.  These numbers all have values >= 512 (see the
        kernel source file arch/x86/entry/syscalls/syscall_64.tbl), which
        allows some headroom to add new system call numbers for the base
        x86-64 architecture (for example, as at Linux 4.16, the highest
        x86-64 system call number 332 is for statx()).  In addition, when an
        x32 binary makes makes any system call, bit 30 (0x40000000) is set
        in the system call number, which enables the kernel code to
        implement 32-bit "compatibility mode" in cases where it is needed.

        In order to build programs for the x32 architecture, one must have
        user-space support (i.e., binutils tools built for x32 and libraries
        such as glibc built to this ABI), and in many distributions this
        support is *not* provided. A notable distribution that does support
        x32 is Debian (https://wiki.debian.org/X32Port).

        For further information on the x32 ABI, see
        https://sites.google.com/site/x32abi/ and
        https://lwn.net/Articles/456731/
*/

/* For the x32 ABI, all system call numbers have bit 30 set */

#define X32_SYSCALL_BIT         0x40000000

#define NR_mq_notify_x86_64     244
#define NR_mq_notify_i386       281
#define NR_mq_notify_x32        527     /* Actual syscall will also have
                                           X32_SYSCALL_BIT bit set */

static void
install_filter(void)
{
    /* The following BPF filter causes mq_notify() to fail with a different
       error on each architecture. (mq_notify() is an example of a system
       call that has a different number on each of x86-64, i386, and x32.)
       To make it easy to determine which system call number triggered the
       seccomp failure, return the system call number as the error number
       that will appear in 'errno'. */

    struct sock_filter filter[] = {

        /* [0] Load the architecture value. */

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                (offsetof(struct seccomp_data, arch))),

        /* [1] Are we on x86-64 architecture? If it is not, jump forward
               to test whether this is the i386 architecture. */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 0, 5),

        /* [2] Load the system call number. */

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 (offsetof(struct seccomp_data, nr))),

        /* [3] Is this the x86-64 mq_notify() syscall? */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, NR_mq_notify_x86_64, 0, 1),

        /* [4] It is; make the call fail with an error equal to the
               syscall number. */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | NR_mq_notify_x86_64),

        /* [5] Is this the x32 mq_notify() syscall? If it is not, jump
               forward to allow the syscall. */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 (NR_mq_notify_x32 + X32_SYSCALL_BIT), 0, 5),

        /* [6] It is; make the call fail with an error equal to the
               syscall number. */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | NR_mq_notify_x32),
                /* Return NR_mq_notify_x32 as the error number, rather than
                   (NR_mq_notify_x32 + X32_SYSCALL_BIT), because glibc only
                   considers negative syscall return values in the range
                   [-4096 < retval < 0] to be error values that should be
                   (negated and) copied to 'errno'.  Note also that even
                   allowing for the masking of the X32_SYSCALL_BIT,
                   mq_notify() is an example of a system call that has a
                   different number on x86-64 and x32 (this is *not* the
                   case for all syscalls under these two ABIs). */

        /* [7] Is this the i386 architecture? If it is not, jump forward
               to kill the process. */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_I386, 0, 4),

        /* [8] Load the system call number. */

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 (offsetof(struct seccomp_data, nr))),

        /* [9] Is this the i386 mq_notify() syscall? If it is not, jump
               forward to allow the syscall. */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, NR_mq_notify_i386, 0, 1),

        /* [10] It is; make the call fail with an error equal to the
                syscall number. */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | NR_mq_notify_i386),

        /* [11] Allow the system call. */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        /* [12] Kill the process if the architecture is not one of those
                that we expect. */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),
    };

    struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &prog) == -1)
        errExit("seccomp");
}

int
main(int argc, char **argv)
{
    const char *no_seccomp_filter_var = "NO_SECCOMP_FILTER";

    /* If the NO_SECCOMP_FILTER environment variable is not set, this should
       be the first invocation of the program, so install the seccomp filter */

    if (getenv(no_seccomp_filter_var) == NULL) {
        if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
            errExit("prctl");

        printf("Initial execution; installing seccomp filter\n");
        install_filter();
    }

    /* Print some information about the execution environment */

    printf("\targv[0] = %s; PID = %ld\n", argv[0], (long) getpid());
    char buf[1024];
    snprintf(buf, sizeof(buf), "echo -n '\t'; grep Seccomp /proc/%ld/status",
            (long) getpid());
    system(buf);
    snprintf(buf, sizeof(buf),
            "echo -n '\t'; file -L /proc/%ld/exe | sed 's/,.*//'",
            (long) getpid());
    system(buf);

    /* Call mq_notify() and check the result, which should be failure
       triggered by the seccomp filter. (The EBADF error is for the case
       where the seccomp filter did not fail the system call, and the
       kernel then returned an error because 0 is not a valid message
       queue descriptor.) */

    if (mq_notify(0, NULL) == -1 && errno != EBADF) {
        printf("mq_notify() failed with errno = %d\n", errno);
    } else {
        printf("mq_notify() was not failed by seccomp() filter!\n");
        printf("*** Make sure that the %s environment variable is not set\n",
                no_seccomp_filter_var);
        printf("    when running this program.\n");
        printf("*** Otherwise, check the syscall numbers in kernel headers\n");
        printf("    to see that they are correct\n");
        exit(EXIT_FAILURE);
    }

    /* If there is an argv[1], then execute the program named in argv[1].
       Beforehand, set the NO_SECCOMP_FILTER environment variable to tell
       the next program not to add a (duplicate) seccomp filter */

    if (argc > 1) {
        setenv(no_seccomp_filter_var, "y", 1);

        printf("\n");
        printf("About to exec: %s\n", argv[1]);
        execv(argv[1], &argv[1]);
        errExit("execvp");
    }

    exit(EXIT_SUCCESS);
}
