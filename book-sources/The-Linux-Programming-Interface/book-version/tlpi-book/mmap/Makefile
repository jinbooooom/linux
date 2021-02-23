include ../Makefile.inc

GEN_EXE = anon_mmap mmcat mmcopy t_mmap

LINUX_EXE = t_remap_file_pages

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
