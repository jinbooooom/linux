include ../Makefile.inc

GEN_EXE = t_chown t_stat t_umask t_utime t_utimes

LINUX_EXE = chiflag

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
