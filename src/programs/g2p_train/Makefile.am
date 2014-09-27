pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = g2p_train

g2p_train_SOURCES = \
	FstPathFinder.cpp \
	FstPathFinder.hpp \
	g2p_train.cpp \
	g2p_train.hpp \
	M2MFstAligner.cpp \
	M2MFstAligner.hpp \
	$(top_srcdir)/src/programs/g2p_eval/util.hpp \
	$(top_srcdir)/src/programs/g2p_eval/utf8.h \
	$(top_srcdir)/src/programs/g2p_eval/utf8/checked.h \
	$(top_srcdir)/src/programs/g2p_eval/utf8/unchecked.h \
	$(top_srcdir)/src/programs/g2p_eval/utf8/core.h \
	main.cpp

g2p_train_LDADD = @OPENFST_LIBS@

AM_CFLAGS =@OPENFST_CFLAGS@
