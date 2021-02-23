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

/* gettimeofday.c

   A short program to demonstrate the benefits that the VDSO confers by
   replacing some traditional system calls with a user-space implementation
   in a shared library that is created by the kernel and mapped into the
   process's address space. Only certain system calls--those whose
   functionality can be performed entirely in user space--can be replaced
   with VDSO implementations; the vdso(7) manual page describes which
   functions are implemented in the VDSO on each Linux architecture.

   One system call that is commonly bypassed through an implementation in
   the VDSO is gettimeofday(2), which returns the current time. Obtaining
   the time on a computer is typically a matter of reading the value of a
   dedicated register (the "timestamp counter", TSC) or other hardware
   location. This operation can be performed from user space, but doing so
   requires machine-specific instructions and information, and we normally
   don't want to code such machine-specific dependencies into a user-space
   application. Thus, instead, the kernel (which by definition knows about
   every architecture) provides a system call that executes the necessary
   machine-specific code.  With the existence of the VDSO, there is now an
   alternative: the kernel (which knows the machine specifics of accessing
   the TSC) can provide code in the VDSO that accesses the TSC. Since the
   VDSO is mapped into the process's user-space memory, the process can now
   access the TSC directly from user space (without itself containing any
   machine-specific code to perform that access).

   This program can be used to produce two binaries, one of which calls the
   gettimeofday() wrapper function in the C library while the other
   directly invokes the kernel system call using syscall(2). The choice is
   determined based on whether or not the USE_SYSCALL macro is defined. On
   systems (e.g., x86) where the kernel provides a VDSO implementation of
   gettimeofday() and the C library knows to use that implementation,
   obtaining the time will be a fast user-space operation.  By contrast,
   when the system call is directly invoked via syscall(2) (a binary build
   with "cc -DUSE_SYSCALL"), obtaining the system time will be much slower.

   The program takes one command-line argument, which specifies the number
   times to loop fetching the system time. Try running the two different
   binaries with the same argument and timing the results using time(1).
   For example:

        time ./syscall_gettimeofday 10000000
        time ./vdso_gettimeofday    10000000
*/
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <sys/syscall.h>
#include <sys/time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct timeval curr;
    long lim;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s loop-count\n", argv[0]);

    lim = atoi(argv[1]);

    for (int j = 0; j < lim; j++) {
#ifdef USE_SYSCALL
        if (syscall(__NR_gettimeofday, &curr, NULL) == -1)
            errExit("gettimeofday");
#else
        if (gettimeofday(&curr, NULL) == -1)
            errExit("gettimeofday");
#endif
    }

    exit(EXIT_SUCCESS);
}
