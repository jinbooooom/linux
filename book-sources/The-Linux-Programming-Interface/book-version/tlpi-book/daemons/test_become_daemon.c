/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 37 */

#include "become_daemon.h"
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    becomeDaemon(0);

    /* Normally a daemon would live forever; we just sleep for a while */

    sleep((argc > 1) ? getInt(argv[1], GN_GT_0, "sleep-time") : 20);

    exit(EXIT_SUCCESS);
}
