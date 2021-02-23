#!/bin/sh
# Build a program using a static library
#

cc -g -c mod1.c mod2.c mod3.c
ar r libdemo.a mod1.o mod2.o mod3.o

cc -g -o prog_static prog.c libdemo.a
# Or: cc -g -o prog_static prog.c -L. -ldemo
