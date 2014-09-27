pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = cdcn_norm

cdcn_norm_SOURCES = cepdist.c correction.c header.h initialize.c main.c max_q.c mmse_x.c norm_utt.c readdistribution.c resfft.c rsfft.c two_D_ize.c 

LDADD = \
	$(top_builddir)/src/libs/libio/libio.la \
	$(top_builddir)/src/libs/libmodinv/libmodinv.la \
	$(top_builddir)/src/libs/libcommon/libcommon.la \
	$(top_builddir)/src/libs/libclust/libclust.la \
	$(top_builddir)/src/libs/libmllr/libmllr.la

AM_CFLAGS =-I$(top_srcdir)/include

