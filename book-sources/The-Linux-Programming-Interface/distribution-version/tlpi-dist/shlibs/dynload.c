/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2020.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 42-1 */

/* dynload.c

   Usage: dynload library-path function-name

   Demonstrate dynamic loading of libraries. The program loads the
   named library and then executes the named function in that library.
*/
#include <dlfcn.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    void *libHandle;            /* Handle for shared library */
    void (*funcp)(void);        /* Pointer to function with no arguments */
    const char *err;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s lib-path func-name\n", argv[0]);

    /* Load the shared library and get a handle for later use */

    libHandle = dlopen(argv[1], RTLD_LAZY);
    if (libHandle == NULL)
        fatal("dlopen: %s", dlerror());

    /* Search library for symbol named in argv[2] */

    (void) dlerror();                           /* Clear dlerror() */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    funcp = (void (*)(void)) dlsym(libHandle, argv[2]);
#pragma GCC diagnostic pop

    /* In the book, instead of the preceding line, the code uses a
       rather clumsy looking cast of the form:

           *(void **) (&funcp) = dlsym(libHandle, argv[2]);

       This was done because the ISO C standard does not require compilers
       to allow casting of pointers to functions back and forth to 'void *'.
       (See TLPI pages 863-864.) SUSv3 TC1 and SUSv4 accepted the ISO C
       requirement and proposed the clumsy cast as the workaround. However,
       the 2013 Technical Corrigendum to SUSv4 requires implementations
       to support casts of the more natural form (now) used in the code
       above. However, various current compilers (e.g., gcc with the
       '-pedantic' flag) may still complain about such casts. Therefore,
       we use a gcc pragma to disable the warning.

       Note that this pragma is available only since gcc 4.6, released in
       2010. If you are using an older compiler, the pragma will generate
       an error. In that case, simply edit this program to remove the
       lines above that begin with '#pragma".

       See also the erratum note for page 864 at
       http://www.man7.org/tlpi/errata/. */

    err = dlerror();
    if (err != NULL)
        fatal("dlsym: %s", err);

    /* Try calling the address returned by dlsym() as a function
       that takes no arguments */

    (*funcp)();

    dlclose(libHandle);                         /* Close the library */

    exit(EXIT_SUCCESS);
}
