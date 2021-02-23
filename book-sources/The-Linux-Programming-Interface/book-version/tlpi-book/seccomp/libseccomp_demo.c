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

/* libseccomp_demo.c
*/
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <seccomp.h>
#include "tlpi_hdr.h"

int main(int argc, char *argv[])
{
    int rc;
    scmp_filter_ctx ctx;

    /* Create seccomp filter state that allows by default */

    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (ctx == NULL)
        fatal("seccomp_init() failed");

    /* Cause clone() and fork() to fail, each with different errors */

    rc = seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(clone), 0);
    if (rc < 0)
        errExitEN(-rc, "seccomp_rule_add");

    rc = seccomp_rule_add(ctx, SCMP_ACT_ERRNO(ENOTSUP), SCMP_SYS(fork), 0);
    if (rc < 0)
        errExitEN(-rc, "seccomp_rule_add");

    /* Export the pseudofilter code and BPF binary code,
       each to different file descriptors (if they are open) */

    seccomp_export_pfc(ctx, 5);
    seccomp_export_bpf(ctx, 6);

    /* Install the seccomp filter into the kernel */

    rc = seccomp_load(ctx);
    if (rc < 0)
        errExitEN(-rc, "seccomp_load");

    /* Free the user-space seccomp filter state */

    seccomp_release(ctx);

    if (fork() != -1)
        fprintf(stderr, "fork() succeeded?!\n");
    else
        perror("fork");

    exit(EXIT_SUCCESS);
}
