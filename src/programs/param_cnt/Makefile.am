pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = param_cnt

param_cnt_SOURCES = cb_cnt.c cb_cnt.h cnt_fn.h enum_corpus.c enum_corpus.h main.c param_cnt.c param_cnt.h parse_cmd_ln.c parse_cmd_ln.h phone_cnt.c phone_cnt.h ts_cnt.c ts_cnt.h 

LDADD = \
	$(top_builddir)/src/libs/libio/libio.la \
	$(top_builddir)/src/libs/libmodinv/libmodinv.la \
	$(top_builddir)/src/libs/libcommon/libcommon.la \
	$(top_builddir)/src/libs/libclust/libclust.la \
	$(top_builddir)/src/libs/libmllr/libmllr.la

AM_CFLAGS =-I$(top_srcdir)/include

