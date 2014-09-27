pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = mk_mdef_gen

mk_mdef_gen_SOURCES = hash.c hash.h heap.c heap.h main.c mk_mdef_gen.c mk_untied.h parse_cmd_ln.c parse_cmd_ln.h 

LDADD = \
	$(top_builddir)/src/libs/libio/libio.la \
	$(top_builddir)/src/libs/libmodinv/libmodinv.la \
	$(top_builddir)/src/libs/libcommon/libcommon.la \
	$(top_builddir)/src/libs/libclust/libclust.la \
	$(top_builddir)/src/libs/libmllr/libmllr.la

AM_CFLAGS =-I$(top_srcdir)/include

