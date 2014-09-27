noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = \
	 acmod_set.c \
	 best_q.c \
	 ck_seg.c \
	 cvt2triphone.c \
	 dtree.c \
	 heap.c \
	 itree.c \
	 lexicon.c \
	 mk_phone_list.c \
	 mk_phone_seq.c \
	 mk_sseq.c \
	 mk_trans_seq.c \
	 mk_ts2ci.c \
	 quest.c \
	 remap.c \
	 state_seq.c \
	 ts2cb.c \
	 vector.c \
	 was_added.c

AM_CFLAGS =-I$(top_srcdir)/include

libcommon_la_LIBADD = $(top_builddir)/src/libs/libclust/libclust.la
