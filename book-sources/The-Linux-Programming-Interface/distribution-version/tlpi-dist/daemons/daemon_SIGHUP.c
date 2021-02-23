/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 37-3 */

/* daemon_SIGHUP.c

   Demonstrate the use of SIGHUP as a mechanism to tell a daemon to
   reread its configuration file and reopen its log file.

   In the version of this code printed in the book, logOpen(), logClose(),
   logMessage(), and readConfigFile() were omitted for brevity. The version
   of the code in this file is complete, and can be compiled and run.
*/
#include <sys/stat.h>
#include <signal.h>
#include "become_daemon.h"
#include "tlpi_hdr.h"

static const char *LOG_FILE = "/tmp/ds.log";
static const char *CONFIG_FILE = "/tmp/ds.conf";

#include <time.h>
#include <stdarg.h>

static FILE *logfp;                 /* Log file stream */

/* Write a message to the log file. Handle variable length argument
   lists, with an initial format string (like printf(3), but without
   a trailing newline). Precede each message with a timestamp. */

static void
logMessage(const char *format, ...)
{
    va_list argList;
    const char *TIMESTAMP_FMT = "%F %X";        /* = YYYY-MM-DD HH:MM:SS */
#define TS_BUF_SIZE sizeof("YYYY-MM-DD HH:MM:SS")       /* Includes '\0' */
    char timestamp[TS_BUF_SIZE];
    time_t t;
    struct tm *loc;

    t = time(NULL);
    loc = localtime(&t);
    if (loc == NULL ||
           strftime(timestamp, TS_BUF_SIZE, TIMESTAMP_FMT, loc) == 0)
        fprintf(logfp, "???Unknown time????: ");
    else
        fprintf(logfp, "%s: ", timestamp);

    va_start(argList, format);
    vfprintf(logfp, format, argList);
    fprintf(logfp, "\n");
    va_end(argList);
}

/* Open the log file 'logFilename' */

static void
logOpen(const char *logFilename)
{
    mode_t m;

    m = umask(077);
    logfp = fopen(logFilename, "a");
    umask(m);

    /* If opening the log fails we can't display a message... */

    if (logfp == NULL)
        exit(EXIT_FAILURE);

    setbuf(logfp, NULL);                    /* Disable stdio buffering */

    logMessage("Opened log file");
}

/* Close the log file */

static void
logClose(void)
{
    logMessage("Closing log file");
    fclose(logfp);
}

/* (Re)initialize from configuration file. In a real application
   we would of course have some daemon initialization parameters in
   this file. In this dummy version, we simply read a single line
   from the file and write it to the log. */

static void
readConfigFile(const char *configFilename)
{
    FILE *configfp;
#define SBUF_SIZE 100
    char str[SBUF_SIZE];

    configfp = fopen(configFilename, "r");
    if (configfp != NULL) {                 /* Ignore nonexistent file */
        if (fgets(str, SBUF_SIZE, configfp) == NULL)
            str[0] = '\0';
        else
            str[strlen(str) - 1] = '\0';    /* Strip trailing '\n' */
        logMessage("Read config file: %s", str);
        fclose(configfp);
    }
}

static volatile sig_atomic_t hupReceived = 0;
                        /* Set nonzero on receipt of SIGHUP */

static void
sighupHandler(int sig)
{
    hupReceived = 1;
}

int
main(int argc, char *argv[])
{
    const int SLEEP_TIME = 15;      /* Time to sleep between messages */
    int count = 0;                  /* Number of completed SLEEP_TIME intervals */
    int unslept;                    /* Time remaining in sleep interval */
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sighupHandler;
    if (sigaction(SIGHUP, &sa, NULL) == -1)
        errExit("sigaction");

    if (becomeDaemon(0) == -1)
        errExit("becomeDaemon");

    logOpen(LOG_FILE);
    readConfigFile(CONFIG_FILE);

    unslept = SLEEP_TIME;

    for (;;) {
        unslept = sleep(unslept);       /* Returns > 0 if interrupted */

        if (hupReceived) {              /* If we got SIGHUP... */
            hupReceived = 0;            /* Get ready for next SIGHUP */
            logClose();
            logOpen(LOG_FILE);
            readConfigFile(CONFIG_FILE);
        }

        if (unslept == 0) {             /* On completed interval */
            count++;
            logMessage("Main: %d", count);
            unslept = SLEEP_TIME;       /* Reset interval */
        }
    }
}
