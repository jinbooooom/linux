/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Listing 22-1 */

/* signal.c

   An implementation of signal() using sigaction().

   Compiling with "-DOLD_SIGS" provides the older, unreliable signal handler
   semantics (which are still the default on some System V derivatives).
*/
/* Some UNIX implementations follow the BSD signal() semantics, including
   Linux, Tru64, and of course the BSD derivatives. Others, such as Solaris,
   follow the System V semantics. We'll conditionally override signal() on
   platforms following the System V semantics. For the other implementations,
   we'll provide a dummy source file that doesn't implement signal().
*/
#if defined(__sun) || defined(__sgi)
#include <signal.h>

typedef void (*sighandler_t)(int);

sighandler_t
signal(int sig, sighandler_t handler)
{
    struct sigaction newDisp, prevDisp;

    newDisp.sa_handler = handler;
    sigemptyset(&newDisp.sa_mask);
#ifdef OLD_SIGNAL
    newDisp.sa_flags = SA_RESETHAND | SA_NODEFER;
#else
    newDisp.sa_flags = SA_RESTART;
#endif

    if (sigaction(sig, &newDisp, &prevDisp) == -1)
        return SIG_ERR;
    else
        return prevDisp.sa_handler;
}

#else
/* The following declaration is provided purely to avoid gcc's
   "warning: ISO C forbids an empty source file" warnings. */

extern int signalDummyVar;

#endif
