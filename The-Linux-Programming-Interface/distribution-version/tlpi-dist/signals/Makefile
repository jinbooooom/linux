include ../Makefile.inc

GEN_EXE = catch_rtsigs demo_SIGFPE ignore_pending_sig intquit \
	nonreentrant ouch sig_receiver sig_sender sig_speed_sigsuspend \
	sigmask_longjmp sigmask_siglongjmp \
	t_kill t_sigaltstack t_sigsuspend t_sigqueue t_sigwaitinfo

LINUX_EXE = signalfd_sigval

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

#
# The following example programs make use of signal() (instead of the
# preferred sigaction()) to establish signal handlers.
# By default, signal() on Linux follows the BSD semantics, establishing 
# a reliable signal handler. This is because _BSD_SOURCE is enabled
# bt default in <features.h>. However, in ../Makefile.inc, we define 
# CFLAGS to include _XOPEN_SOURCE, and this has the effect of disabling
# _BSD_SOURCE. To ensure that we get the default (BSD) semantics
# we here explicitly define _BSD_SOURCE when compiling these programs.
#

ouch : ouch.c
	${CC} -D_BSD_SOURCE -o $@ ouch.c ${CFLAGS} ${IMPL_LDLIBS}

intquit : intquit.c
	${CC} -D_BSD_SOURCE -o $@ intquit.c ${CFLAGS} ${IMPL_LDLIBS}

sig_receiver : sig_receiver.c
	${CC} -D_BSD_SOURCE -o $@ sig_receiver.c ${CFLAGS} ${IMPL_LDLIBS}

nonreentrant : nonreentrant.o
	${CC} -o $@ nonreentrant.o ${CFLAGS} ${IMPL_LDLIBS} ${LINUX_LIBCRYPT}

sigmask_siglongjmp.o : sigmask_longjmp.c
	${CC} -o $@ -DUSE_SIGSETJMP -c sigmask_longjmp.c ${CFLAGS}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
