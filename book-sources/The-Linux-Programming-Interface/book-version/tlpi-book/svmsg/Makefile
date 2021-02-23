include ../Makefile.inc

GEN_EXE = svmsg_chqbytes svmsg_file_client svmsg_file_server \
	svmsg_create svmsg_receive svmsg_rm svmsg_send 

LINUX_EXE = svmsg_info svmsg_ls 

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

svmsg_file_client.o svmsg_file_server.o : svmsg_file.h

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
