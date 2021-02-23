include ../Makefile.inc

GEN_EXE = 

LINUX_EXE = cap_launcher cap_text check_password_caps \
	    demo_file_caps \
	    show_secbits \
	    t_cap_get_file t_cap_get_pid t_cap_set_file
# Note: view_cap_xattr is not included in LINUX_EXE, because it depends
# on having a fairly recent Linux kernel (4.14 or later)

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

LDLIBS = ${IMPL_LDLIBS} ${LINUX_LIBCAP} ${LINUX_LIBCRYPT}
        # Some programs that we build here need libcap and libcrypt

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
