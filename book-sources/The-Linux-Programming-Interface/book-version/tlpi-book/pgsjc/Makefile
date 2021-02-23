include ../Makefile.inc

GEN_EXE = catch_SIGHUP disc_SIGHUP job_mon \
	orphaned_pgrp_SIGHUP handling_SIGTSTP t_setsid

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
