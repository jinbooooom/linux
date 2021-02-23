include ../Makefile.inc

GEN_EXE = sched_set sched_view t_setpriority 

LINUX_EXE = demo_sched_fifo t_sched_setaffinity t_sched_getaffinity

EXE = ${GEN_EXE} ${LINUX_EXE}

LDLIBS = ${IMPL_LDLIBS} #CF[ ${LINUX_LIBRT} ]

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
