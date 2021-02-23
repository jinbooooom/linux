#!/bin/sh
#
# Build a program using a shared library
#
# Objects for library must be compiled with PIC (position 
# independent code) generation

gcc -g -c -fPIC -Wall mod1.c mod2.c mod3.c

# Create library with "real name" (libXYZ.so.maj#.min#.rel#)
# and embed the "soname" (libXYZ.so.maj#)

gcc -g -shared -Wl,-soname,libdemo.so.1 -o libdemo.so.1.0.1 \
		mod1.o mod2.o mod3.o
		
# Create "soname" (libXYZ.so.maj# - a symbolic link to real name)
	
ln -sf libdemo.so.1.0.1 libdemo.so.1

# Create "linker name" (libXYZ.so - a symbolic link to "soname")

ln -sf libdemo.so.1 libdemo.so

# Build program by linking against "linker name"

cc -g -Wall -c prog.c
cc -g -o prog_so prog.o -L. -ldemo
