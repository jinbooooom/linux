#!/bin/sh
#
# Build a program without using libraries

cc -c -g prog.c mod1.c mod2.c mod3.c
cc -g -o prog_nolib prog.o mod1.o mod2.o mod3.o
