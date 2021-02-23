include ../Makefile.inc

GEN_EXE = demo_sigio poll_pipes select_mq self_pipe t_select

LINUX_EXE = epoll_flags_fork epoll_input multithread_epoll_wait

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

multithread_epoll_wait: multithread_epoll_wait.o
	${CC} -o $@ multithread_epoll_wait.o \
		${CFLAGS} ${IMPL_LDLIBS} ${IMPL_THREAD_FLAGS}

