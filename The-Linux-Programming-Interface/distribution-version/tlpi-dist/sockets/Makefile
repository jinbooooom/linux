include ../Makefile.inc

GEN_EXE = i6d_ucase_sv i6d_ucase_cl \
	id_echo_cl id_echo_sv \
	is_echo_cl is_echo_sv is_echo_inetd_sv is_echo_v2_sv \
	is_seqnum_sv is_seqnum_cl is_seqnum_v2_sv is_seqnum_v2_cl \
	socknames t_gethostbyname t_getservbyname \
	ud_ucase_sv ud_ucase_cl \
	us_xfr_cl us_xfr_sv us_xfr_v2_cl us_xfr_v2_sv

LINUX_EXE = list_host_addresses \
	scm_cred_recv scm_cred_send \
	scm_multi_recv scm_multi_send \
	scm_rights_recv scm_rights_send \
	us_abstract_bind

EXE = ${GEN_EXE} ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

i6d_ucase_sv.o i6d_ucase_cl.o : i6d_ucase.h 

id_echo_cl.o id_echo_sv.o : id_echo.h 

is_seqnum_sv.o is_seqnum_cl.o : is_seqnum.h 

is_seqnum_v2_sv.o is_seqnum_v2_cl.o : is_seqnum_v2.h 

scm_cred_recv.o scm_cred_send.o : scm_cred.h

scm_multi_recv.o scm_multi_send.o : scm_multi.h

scm_rights_recv.o scm_rights_send.o : scm_rights.h


us_xfr_sv.o us_xfr_cl.o : us_xfr.h 

us_xfr_v2_sv.o us_xfr_v2_cl.o : us_xfr_v2.h 

ud_ucase_sv.o ud_ucase_cl.o : ud_ucase.h 

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}

${EXE} : ${TLPI_LIB}		# True as a rough approximation
