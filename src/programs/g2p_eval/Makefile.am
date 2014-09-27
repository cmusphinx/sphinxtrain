pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = phonetisaurus-g2p

phonetisaurus_g2p_SOURCES = \
	main.cpp \
	phonetisaurus-g2p.hpp \
	phonetisaurus-g2p.cpp \
	util.hpp \
	Phonetisaurus.cpp \
	Phonetisaurus.hpp \
	$(top_srcdir)/src/programs/g2p_train/FstPathFinder.cpp \
	$(top_srcdir)/src/programs/g2p_train/FstPathFinder.hpp \
	utf8.h \
	utf8/checked.h \
	utf8/unchecked.h \
	utf8/core.h

phonetisaurus_g2p_LDADD = @OPENFST_LIBS@

AM_CFLAGS =@OPENFST_CFLAGS@
