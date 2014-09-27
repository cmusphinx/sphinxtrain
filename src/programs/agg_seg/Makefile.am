pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = agg_seg

agg_seg_SOURCES = \
	 add_seg.h \
	 agg_all_seg.c \
	 agg_all_seg.h \
	 agg_phn_seg.c \
	 agg_phn_seg.h \
	 agg_st_seg.c \
	 agg_st_seg.h \
	 cnt_phn_seg.c \
	 cnt_phn_seg.h \
	 cnt_st_seg.c \
	 cnt_st_seg.h \
	 main.c \
	 mk_seg.c \
	 mk_seg.h \
	 parse_cmd_ln.c \
	 parse_cmd_ln.h

LDADD = \
	$(top_builddir)/src/libs/libio/libio.la \
	$(top_builddir)/src/libs/libmodinv/libmodinv.la \
	$(top_builddir)/src/libs/libcommon/libcommon.la \
	$(top_builddir)/src/libs/libclust/libclust.la \
	$(top_builddir)/src/libs/libmllr/libmllr.la

AM_CFLAGS =-I$(top_srcdir)/include

