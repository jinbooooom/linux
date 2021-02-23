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

/* seccomp_arg64.c

   Some system call arguments can be 64 bits in size, for example pointers
   and the 'off_t' argument of system calls such as lseek(). The
   'seccomp_data' structure allows for this: the elements of 'args' are each
   64 bits in size.  However, the BBF accumulator register is only 32 bits
   in size.  Therefore, 64-bit system call arguments must be dealt with in
   two pieces.  This program provides a simple example of how to do this.

   The program applies a seccomp filter that causes lseek() calls that
   specify an 'offset' value greater than 1000 to fail with an error in
   'errno'. The error number varies, depending on whether or not there
   'offset' argument was bigger than 32 bits.

   See also the kernel source file samples/seccomp/bpf-helper.h
   for some useful helper macros for dealing with 64-bit arguments.
*/
#define _GNU_SOURCE
#include <stddef.h>
#include <fcntl.h>
#include <linux/audit.h>
#include <sys/syscall.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include "tlpi_hdr.h"

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
seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(__NR_seccomp, operation, flags, args);
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

        /* Allow system calls other than lseek() */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_lseek, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        /* Load top 4 bytes of 'offset' argument; fail with errno==2 if
           these bytes are nonzero. The code here assumes a little-endian
           architecture (i.e., that the fist 4 bytes are the least
           significant bytes of the 64-bit argument and the following
           4 bytes are the most significant bytes). There are some macros
           in the kernel source file samples/seccomp/bpf-helper.h that show
           how endianess differences can be abstracted away when dealing
           with 64-bit arguments. */

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 (offsetof(struct seccomp_data, args[1]) + sizeof(__u32))),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | 2),

        /* Load bottom 4 bytes of 'offset' argument; fail with errno==1
           if the value is > 1000; otherwise allow the system call */

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 (offsetof(struct seccomp_data, args[1]))),
        BPF_JUMP(BPF_JMP | BPF_JGT | BPF_K, 1000, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | 1),

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    if (seccomp(SECCOMP_SET_MODE_FILTER, 0, &prog) == -1)
        errExit("seccomp");
    /* On Linux 3.16 and earlier, we must instead use:
            if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog))
                errExit("prctl-PR_SET_SECCOMP");
    */
}

static void
seek_test(int fd, off_t offset)
{
    off_t ret;

    printf("Seek to byte %lld: ", (long long) offset);
    ret = lseek(fd, offset, SEEK_SET);
    if (ret == 0)
        printf("succeeded\n");
    else
        printf("failed with errno = %d\n", errno);
}

int
main(int argc, char **argv)
{
    int fd;

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
        errExit("prctl");

    install_filter();

    fd = open("/dev/zero", O_RDWR);
    if (fd == -1)
        errExit("open");

    seek_test(fd, 0);
    seek_test(fd, 10000);
    seek_test(fd, 0x100000001);

    exit(EXIT_SUCCESS);
}
