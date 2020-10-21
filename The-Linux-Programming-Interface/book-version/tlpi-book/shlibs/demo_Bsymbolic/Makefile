TARGETS = prog libfoo1.so libfoo2.so libfoo3.so

all: ${TARGETS}

run:
	LD_LIBRARY_PATH=. ./prog

prog: prog.o libfoo1.so libfoo2.so libfoo3.so
	cc -g -o prog prog.c -L. -lfoo1 -lfoo2 -lfoo3

libfoo1.so:
	cc -g -shared -fPIC -o libfoo1.so foo1.c

libfoo2.so:
	cc -g -Wl,-Bsymbolic -shared -fPIC -o libfoo2.so foo2.c

libfoo3.so:
	cc -g -Wl,-Bsymbolic -Wl,-allow-shlib-undefined \
    		-shared -fPIC -o libfoo3.so foo3.c

clean:
	${RM} ${TARGETS} *.o

