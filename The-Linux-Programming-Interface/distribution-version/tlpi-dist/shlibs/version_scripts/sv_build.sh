#!/bin/sh
#
# sv_build.sh
#
set -v

# Build version 1 of shared library 

gcc -g -c -fPIC -Wall sv_lib_v1.c
gcc -g -shared -o libsv.so sv_lib_v1.o -Wl,--version-script,sv_v1.map

gcc -g -o p1 sv_prog.c libsv.so

LD_LIBRARY_PATH=. ./p1

# Build version 2 of shared library 

gcc -g -c -fPIC -Wall sv_lib_v2.c
gcc -g -shared -o libsv.so sv_lib_v2.o -Wl,--version-script,sv_v2.map

gcc -g -o p2 sv_prog.c libsv.so

LD_LIBRARY_PATH=. ./p2
LD_LIBRARY_PATH=. ./p1
