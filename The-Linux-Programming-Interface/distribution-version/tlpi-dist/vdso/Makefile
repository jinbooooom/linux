include ../Makefile.inc

GEN_EXE = 

LINUX_EXE = vdso_gettimeofday syscall_gettimeofday

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

vdso_gettimeofday : gettimeofday.c
	${CC} -o $@ gettimeofday.c ${CFLAGS} ${IMPL_LDLIBS}

syscall_gettimeofday : gettimeofday.c
	${CC} -DUSE_SYSCALL -o $@ gettimeofday.c ${CFLAGS} ${IMPL_LDLIBS}

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
