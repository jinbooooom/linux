#!/bin/sh
#
# vis_build.sh
#
# Show how versions scripts can be used to control symbol visibility.
#
set -v

# In the following, vis_comm() is accidentally exported by the 
# shared library

gcc -g -c -fPIC -Wall vis_comm.c vis_f1.c vis_f2.c
gcc -g -shared -o vis.so vis_comm.o vis_f1.o vis_f2.o
readelf --syms --use-dynamic vis.so | grep vis_

# Now we use a version script to control exactly which symbols
# are exported by the library

gcc -g -c -fPIC -Wall vis_comm.c vis_f1.c vis_f2.c
gcc -g -shared -o vis.so vis_comm.o vis_f1.o vis_f2.o \
        -Wl,--version-script,vis.map
readelf --syms --use-dynamic vis.so | grep vis_
