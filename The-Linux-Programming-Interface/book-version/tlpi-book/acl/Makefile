include ../Makefile.inc

GEN_EXE = 

LINUX_EXE = acl_update acl_view

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBACL}
	# All of the programs in this directory need the 
	# ACL library, libacl.

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
