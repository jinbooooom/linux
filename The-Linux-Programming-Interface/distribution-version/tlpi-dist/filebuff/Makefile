include ../Makefile.inc

GEN_EXE = copy mix23_linebuff mix23io \
	  write_bytes \
	  write_bytes_fdatasync \
	  write_bytes_fsync \
	  write_bytes_o_sync

LINUX_EXE = direct_read

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

write_bytes_fdatasync : write_bytes.c
	${CC} -DUSE_FDATASYNC -o $@ write_bytes.c ${CFLAGS} ${IMPL_LDLIBS}

write_bytes_fsync : write_bytes.c
	${CC} -DUSE_FSYNC -o $@ write_bytes.c ${CFLAGS} ${IMPL_LDLIBS}

write_bytes_o_sync : write_bytes.c
	${CC} -DUSE_O_SYNC -o $@ write_bytes.c ${CFLAGS} ${IMPL_LDLIBS}

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
