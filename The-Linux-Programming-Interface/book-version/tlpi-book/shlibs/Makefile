include ../Makefile.inc

GEN_EXE = dynload

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

dynload : dynload.o
	${CC} -o $@ dynload.o ${CFLAGS} ${IMPL_LDLIBS} ${LINUX_LIBDL}

clean :
	${RM} ${EXE} *.o *.so.*

${EXE} : ${TLPI_LIB}		# True as a rough approximation
