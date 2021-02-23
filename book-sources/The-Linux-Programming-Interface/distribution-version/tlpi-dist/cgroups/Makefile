include ../Makefile.inc

GEN_EXE = alloc_mem fork_bomb

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

alloc_mem: alloc_mem.o
	${CC} -o $@ alloc_mem.o \
		${CFLAGS} ${IMPL_LDLIBS} ${IMPL_THREAD_FLAGS}
