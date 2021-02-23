include ../Makefile.inc

GEN_EXE =

LINUX_EXE = demo_inotify dnotify inotify_dtree rand_dtree

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allftw : ${FTW_EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
