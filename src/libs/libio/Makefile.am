noinst_LTLIBRARIES = libio.la

libio_la_SOURCES = \
	 pset_io.c \
	 corpus.c \
	 model_def_io.c \
	 s3acc_io.c \
	 s3cb2mllr_io.c \
	 s3gau_full_io.c \
	 s3gau_io.c \
	 s3io.c \
	 s3lamb_io.c \
	 s3map_io.c \
	 s3mixw_io.c \
	 s3phseg_io.c \
	 s3tmat_io.c \
	 s3ts2cb_io.c \
	 segdmp.c \
	 swap.c \
	 topo_read.c

AM_CFLAGS =-I$(top_srcdir)/include

libio_la_LIBADD = $(top_builddir)/src/libs/libcommon/libcommon.la

