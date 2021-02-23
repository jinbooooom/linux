/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 40-2 */

/* dump_utmpx.c

   Display the contents of the utmp-style file named on the command line.

   This version of the program differs from that which appears in the book in
   that it prints extra information from each utmpx record.

   This program is Linux-specific.
*/
#define _GNU_SOURCE
#include <time.h>
#include <utmpx.h>
#include <paths.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct utmpx *ut;
    struct in_addr in;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [utmp-pathname]\n", argv[0]);

    if (argc > 1)               /* Use alternate file if supplied */
        if (utmpxname(argv[1]) == -1)
            errExit("utmpxname");

    setutxent();

    printf("user     type            PID line   id  host     ");
    printf("term exit session  address         date/time\n");

    while ((ut = getutxent()) != NULL) {        /* Sequential scan to EOF */
        printf("%-8s ", ut->ut_user);
        printf("%-9.9s ",
                (ut->ut_type == EMPTY) ?         "EMPTY" :
                (ut->ut_type == RUN_LVL) ?       "RUN_LVL" :
                (ut->ut_type == BOOT_TIME) ?     "BOOT_TIME" :
                (ut->ut_type == NEW_TIME) ?      "NEW_TIME" :
                (ut->ut_type == OLD_TIME) ?      "OLD_TIME" :
                (ut->ut_type == INIT_PROCESS) ?  "INIT_PR" :
                (ut->ut_type == LOGIN_PROCESS) ? "LOGIN_PR" :
                (ut->ut_type == USER_PROCESS) ?  "USER_PR" :
                (ut->ut_type == DEAD_PROCESS) ?  "DEAD_PR" : "???");
        printf("(%1d) ", ut->ut_type);
        printf("%5ld %-6.6s %-3.5s %-9.9s ", (long) ut->ut_pid,
                ut->ut_line, ut->ut_id, ut->ut_host);
        printf("%3d %3d ", ut->ut_exit.e_termination, ut->ut_exit.e_exit);
        printf("%8ld ", (long) ut->ut_session);

        /* Display IPv4 address */

        in.s_addr = ut->ut_addr_v6[0];
        printf(" %-15.15s ", inet_ntoa(in));
        time_t tv_sec = ut->ut_tv.tv_sec;
        printf("%s", ctime((time_t *) &tv_sec));
    }

    endutxent();
    exit(EXIT_SUCCESS);
}
