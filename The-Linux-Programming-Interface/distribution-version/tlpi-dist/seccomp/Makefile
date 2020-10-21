include ../Makefile.inc

GEN_EXE = 

LINUX_EXE = dump_seccomp_filter \
	    libseccomp_demo \
	    seccomp_arg64 \
	    seccomp_bench \
	    seccomp_control_open seccomp_deny_open \
	    seccomp_launch \
	    seccomp_perf \
	    seccomp_user_notification

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

libseccomp_demo : libseccomp_demo.c
	${CC} -o $@ libseccomp_demo.c ${CFLAGS} ${IMPL_LDLIBS} -lseccomp

# See the seccomp_multiarch.c source code for information on the library
# that must be installed before building seccomp_multiarch_i386

seccomp_multiarch : seccomp_multiarch.o
	${CC} -o $@ seccomp_multiarch.o ${CFLAGS} ${IMPL_LDLIBS} ${LINUX_LIBRT}

seccomp_multiarch_i386 : seccomp_multiarch.c
	${CC} -m32 -o $@ seccomp_multiarch.c ${CFLAGS} \
		${IMPL_LDLIBS} ${LINUX_LIBRT}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
