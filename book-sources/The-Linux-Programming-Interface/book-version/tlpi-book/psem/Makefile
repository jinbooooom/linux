include ../Makefile.inc

GEN_EXE = psem_getvalue psem_create psem_post psem_unlink \
	  psem_timedwait psem_trywait psem_wait thread_incr_psem

LINUX_EXE =

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

CFLAGS = ${IMPL_CFLAGS} ${IMPL_THREAD_FLAGS}
LDLIBS = ${IMPL_LDLIBS} ${IMPL_THREAD_FLAGS}
	# POSIX semaphores need the NPTL thread library on Linux

# psem_timedwait uses clock_gettime() which is in librt
psem_timedwait: psem_timedwait.o
	${CC} -o $@ psem_timedwait.o ${CFLAGS} ${LDLIBS} ${LINUX_LIBRT}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
