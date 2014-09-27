pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = cdcn_train

cdcn_train_SOURCES = em_full_vars.c gauss.c header.h main.c parse_cmd_ln.c parse_cmd_ln.h read_backup_distributions.c store_distribution.c vq.c 

LDADD = \
	$(top_builddir)/src/libs/libio/libio.la \
	$(top_builddir)/src/libs/libmodinv/libmodinv.la \
	$(top_builddir)/src/libs/libcommon/libcommon.la \
	$(top_builddir)/src/libs/libclust/libclust.la \
	$(top_builddir)/src/libs/libmllr/libmllr.la

AM_CFLAGS =-I$(top_srcdir)/include
