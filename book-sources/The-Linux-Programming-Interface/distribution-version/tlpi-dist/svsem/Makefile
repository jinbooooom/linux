include ../Makefile.inc

GEN_EXE = svsem_create svsem_demo svsem_mon svsem_op svsem_rm svsem_setall 

LINUX_EXE = svsem_info

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
