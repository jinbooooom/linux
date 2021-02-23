/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 31-1 */

#include <pthread.h>
#include "tlpi_hdr.h"

struct once_struct {            /* Our equivalent of pthread_once_t */
    pthread_mutex_t mtx;
    int called;
};

#define ONCE_INITIALIZER { PTHREAD_MUTEX_INITIALIZER, 0 }

struct once_struct once = ONCE_INITIALIZER;

static int
one_time_init(struct once_struct *once_control, void (*init)(void))
{
    int s;

    s = pthread_mutex_lock(&(once_control->mtx));
    if (s == -1)
        errExitEN(s, "pthread_mutex_lock");

    if (!once_control->called) {
        (*init)();
        once_control->called = 1;
    }

    s = pthread_mutex_unlock(&(once_control->mtx));
    if (s == -1)
        errExitEN(s, "pthread_mutex_unlock");

    return 0;
}

/* Remaining code is for testing one_time_init() */

static void
init_func()
{
    /* We should see this message only once, no matter how many
       times one_time_init() is called */

    printf("Called init_func()\n");
}

static void *
threadFunc(void *arg)
{
    /* The following allows us to verify that even if a single thread calls
       one_time_init() multiple times, init_func() is called only once */

    one_time_init(&once, init_func);
    one_time_init(&once, init_func);
    return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int s;

    /* Create two threads, both of which will call one_time_init() */

    s = pthread_create(&t1, NULL, threadFunc, (void *) 1);
    if (s != 0)
        errExitEN(s, "pthread_create");

    s = pthread_create(&t2, NULL, threadFunc, (void *) 2);
    if (s != 0)
        errExitEN(s, "pthread_create");

    s = pthread_join(t1, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");
    printf("First thread returned\n");

    s = pthread_join(t2, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");
    printf("Second thread returned\n");

    exit(EXIT_SUCCESS);
}
