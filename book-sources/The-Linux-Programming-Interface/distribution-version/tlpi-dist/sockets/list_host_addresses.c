/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 61 */

/* list_host_addresses.c

   List host's network interfaces and IP addresses.
*/
#define _GNU_SOURCE     /* To get definition of NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

int
main(int argc, char *argv[])
{
    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, ignoring loopback interface and
       non-AF_INET* addresses */

    for (; ifaddr != NULL; ifaddr = ifaddr->ifa_next) {

        if (ifaddr->ifa_addr == NULL || strcmp(ifaddr->ifa_name, "lo") == 0)
            continue;

        family = ifaddr->ifa_addr->sa_family;

        if (family != AF_INET && family != AF_INET6)
            continue;

        /* Display interface name and address */

        s = getnameinfo(ifaddr->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                          sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (s != 0) {
            printf("getnameinfo() failed: %s\n", gai_strerror(s));
            exit(EXIT_FAILURE);
        }

        printf("%-16s %s\n", ifaddr->ifa_name, host);
    }

    exit(EXIT_SUCCESS);
}
