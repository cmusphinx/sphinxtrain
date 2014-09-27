pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = bw

bw_SOURCES = accum.c accum.h backward.c backward.h baum_welch.c baum_welch.h forward.c forward.h main.c next_utt_states.c next_utt_states.h train_cmd_ln.c train_cmd_ln.h viterbi.c viterbi.h 

LDADD = \
	$(top_builddir)/src/libs/libio/libio.la \
	$(top_builddir)/src/libs/libmodinv/libmodinv.la \
	$(top_builddir)/src/libs/libcommon/libcommon.la \
	$(top_builddir)/src/libs/libclust/libclust.la \
	$(top_builddir)/src/libs/libmllr/libmllr.la

AM_CFLAGS =-I$(top_srcdir)/include

