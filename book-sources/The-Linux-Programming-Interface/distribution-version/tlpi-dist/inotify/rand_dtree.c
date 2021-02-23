/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter 19 */

/* rand_dtree.c

   A test program to use in conjunction with inotify_dtree.c.

   This program randomly creates, deletes, or renames
   subdirectories underneath the pathname specified in its
   sole command-line argument.
*/
#if ! defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 700
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#include <stdarg.h>
#include <limits.h>
#include <ftw.h>
#include "tlpi_hdr.h"

#define DLIM 60

/* Hack! We can't pass arguments to the function invoked by nftw(),
   so we use these global variables to exchange information with
   the function */

static int dcnt;
static char **dirList = NULL;
static int dlSize = 0;
static const int D_INCR = 1000;

static int
traverseTree(const char *pathname, const struct stat *sb, int tflag,
             struct FTW *ftwbuf)
{
    if (! S_ISDIR(sb->st_mode))
        return 0;

    //printf("%s\n", pathname);

    if (dcnt >= dlSize) {
        dlSize += D_INCR;
        dirList = realloc(dirList, dlSize * sizeof(char *));
        if (dirList == NULL)
            errExit("realloc");
    }

    dirList[dcnt] = strdup(pathname);

    dcnt++;

    return 0;
}

static int
getDirList(const char *pathname)
{
    dcnt = 0;

    if (nftw(pathname, traverseTree, 20, FTW_PHYS) == -1)
        errMsg("nftw: %s", pathname);

    return dcnt;
}

static FILE *logfp = NULL;

static void
logMessage(const char *format, ...)
{
    va_list argList;

    va_start(argList, format);

    if (logfp != NULL)
        vfprintf(logfp, format, argList);

    va_end(argList);
}

static void
usageError(char *pname)
{
    fprintf(stderr, "Usage: %s [options] dirpath {c|d|m}\n\n", pname);
    fprintf(stderr, "Perform random operations in the "
            "directory tree 'dirpath'\n");
    fprintf(stderr, "    c == create directories\n");
    fprintf(stderr, "    d == delete directories\n");
    fprintf(stderr, "    m == rename directories\n\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -l logfile     Record activity in log file\n");
    fprintf(stderr, "    -m maxops      Do at most 'maxops' operations "
            "(default is unlimited)\n");
    fprintf(stderr, "    -s usecs       Sleep 'usecs' microseconds "
            "between each operation\n");
    fprintf(stderr, "    -z stopfile    Immediately stop when the file "
            "'stopfile' is created\n");
    exit(EXIT_FAILURE);
}

#define MARKER_STRING "--"

int
main(int argc, char *argv[])
{
    int j;
    char *to_move;
    int opcnt, maxops;
    char path[PATH_MAX];
    char tfile[PATH_MAX];
    char target[PATH_MAX];
    char spath[PATH_MAX];
    char *p;
    int nslashes;
    int opt;
    int usecs;
    char *stopFile;
    int scnt;
    int s;

    opcnt = 0;

    srandom(0);

    stopFile = NULL;
    maxops = 0;
    usecs = 1;
    while ((opt = getopt(argc, argv, "l:m:s:z:")) != -1) {
        switch (opt) {
        case 's':
            usecs = atoi(optarg);
            break;

        case 'z':
            stopFile = optarg;
            break;

        case 'm':
            maxops = atoi(optarg);
            break;

        case 'l':
            logfp = fopen(optarg, "w+");
            if (logfp == NULL)
                errExit("fopen");
            setbuf(logfp, NULL);
            break;

        default:
            usageError(argv[0]);
        }
    }

    if (optind + 1 >= argc)
        usageError(argv[0]);

    opcnt = 0;

    for (;;) {

        getDirList(argv[optind]);

        switch (argv[optind + 1][0]) {
        case 'c':
            snprintf(path, sizeof(path), "%s/%ld%s%s_%d",
                    dirList[random() % dcnt],
                    (long) getpid() % 100, MARKER_STRING, "cr", opcnt);
            if (strlen(path) > DLIM)
                continue;
            nslashes = 0;
            for (p = path; *p; p++)
                if (*p == '/')
                    nslashes++;

            if (nslashes > 1)
                if (random() % nslashes > 0)
                    continue;

            if (mkdir(path, 0700) == 0)
                logMessage("mkdir: %s\n", path);

            scnt = 1;
            while ((random() % 3) < 2) {
                s = snprintf(spath, sizeof(path), "%s/%ld%s%s%d_%d", path,
                        (long) getpid() % 100,
                        MARKER_STRING, "scr", scnt, opcnt);
                if (s > DLIM)
                    break;

                if (mkdir(spath, 0700) == 0)
                    logMessage("mkdir: %s\n", spath);

                strncpy(path, spath, PATH_MAX);
                path[PATH_MAX - 1] = '\0';
                scnt++;
            }
            break;

        case 'd':
            if (dcnt == 0)
                continue;

            snprintf(path, sizeof(path), "%s", dirList[random() % dcnt]);
            while (strstr(path, MARKER_STRING) != NULL) {

                if (rmdir(path) == -1)
                    break;
                logMessage("rmdir: %s\n", path);

                p = strrchr(path, '/');
                if (p == NULL)
                    break;

                *p = '\0';
            }
            break;

        case 'm':
            if (dcnt < 3)
                continue;

            to_move = dirList[random() % dcnt];

            if (strstr(to_move, MARKER_STRING) != NULL) {
                p = strrchr(to_move, '/');
                snprintf(tfile, sizeof(tfile), "%s", p + 1);
                p = strstr(tfile, "__ren");
                if (p != NULL)
                    *p = '\0';

                s = snprintf(target, sizeof(target), "%s/%s__ren%04d-%ld",
                        dirList[random() % dcnt],
                        tfile, opcnt, (long) getpid());

                if (s > DLIM)
                    break;

                if (rename(to_move, target) == 0)
                    logMessage("rename: %s ==> %s\n", to_move, target);
            }

            break;
        }

        for (j = 0; j < dcnt; j++) {
            free(dirList[j]);
            dirList[j] = NULL;
        }

        opcnt++;

        usleep(usecs);

        if (maxops > 0 && opcnt >= maxops)
            break;

        if (access(stopFile, F_OK) == 0)
            break;
    }

    exit(EXIT_SUCCESS);
}
