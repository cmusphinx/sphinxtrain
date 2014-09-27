pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = map_adapt

map_adapt_SOURCES = cmd_ln.c cmd_ln_defn.h main.c parse_cmd_ln.h 

LDADD = \
	$(top_builddir)/src/libs/libio/libio.la \
	$(top_builddir)/src/libs/libmodinv/libmodinv.la \
	$(top_builddir)/src/libs/libcommon/libcommon.la \
	$(top_builddir)/src/libs/libclust/libclust.la \
	$(top_builddir)/src/libs/libmllr/libmllr.la

AM_CFLAGS =-I$(top_srcdir)/include

