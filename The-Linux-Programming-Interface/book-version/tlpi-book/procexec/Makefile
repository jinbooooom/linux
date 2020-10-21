include ../Makefile.inc

GEN_EXE = acct_on acct_view child_status closeonexec envargs exit_handlers \
	footprint fork_file_sharing fork_sig_sync \
	fork_stdio_buf fork_whos_on_first \
	make_zombie multi_SIGCHLD multi_wait necho orphan \
	pdeath_signal \
	t_execl t_execle t_execve t_execlp t_fork t_system \
	t_vfork vfork_fd_test

LINUX_EXE = demo_clone t_clone acct_v3_view

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation

pdeath_signal: pdeath_signal.o
	${CC} -o $@ pdeath_signal.o \
		${CFLAGS} ${IMPL_LDLIBS} ${IMPL_THREAD_FLAGS}
