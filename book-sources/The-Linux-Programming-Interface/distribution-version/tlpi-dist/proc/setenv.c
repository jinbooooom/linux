/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 6-3 */

/* setenv.c

   An implementation of setenv() and unsetenv() using environ, putenv(),
   and getenv().
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int
unsetenv(const char *name)
{
    extern char **environ;
    char **ep, **sp;
    size_t len;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }

    len = strlen(name);

    for (ep = environ; *ep != NULL; )
        if (strncmp(*ep, name, len) == 0 && (*ep)[len] == '=') {

            /* Remove found entry by shifting all successive entries
               back one element */

            for (sp = ep; *sp != NULL; sp++)
                *sp = *(sp + 1);

            /* Continue around the loop to further instances of 'name' */

        } else {
            ep++;
        }

    return 0;
}

int
setenv(const char *name, const char *value, int overwrite)
{
    char *es;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL ||
            value == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (getenv(name) != NULL && overwrite == 0)
        return 0;

    unsetenv(name);             /* Remove all occurrences */

    es = malloc(strlen(name) + strlen(value) + 2);
                                /* +2 for '=' and null terminator */
    if (es == NULL)
        return -1;

    strcpy(es, name);
    strcat(es, "=");
    strcat(es, value);

    return (putenv(es) != 0) ? -1 : 0;
}

#ifdef TEST_IT

int
main()
{
    if (putenv("TT=xxxxx") != 0)
        perror("putenv");

    system("echo '***** Environment before unsetenv(TT)'; "
            "printenv | grep ^TT");
    system("echo 'Total lines from printenv:' `printenv | wc -l`");

    unsetenv("TT");

    system("echo '***** Environment after unsetenv(TT)'; "
            "printenv | grep ^TT");
    system("echo 'Total lines from printenv:' `printenv | wc -l`");

    setenv("xyz", "one", 1);
    setenv("xyz", "two", 0);
    setenv("xyz2", "222", 0);

    system("echo '***** Environment after setenv() calls'; "
            "printenv | grep ^x");
    system("echo 'Total lines from printenv:' `printenv | wc -l`");

    exit(EXIT_SUCCESS);
}

#endif
