include ../Makefile.inc

GEN_EXE = svshm_attach svshm_create svshm_mon svshm_rm \
	svshm_xfr_reader svshm_xfr_writer 

LINUX_EXE = svshm_info svshm_lock svshm_unlock

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

svshm_xfr_reader.o svshm_xfr_writer.o: svshm_xfr.h

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
