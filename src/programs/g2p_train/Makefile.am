pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = g2p_train

g2p_train_SOURCES = FstPathFinder.cpp  FstPathFinder.hpp  g2p_train.cpp  M2MFstAligner.cpp  M2MFstAligner.hpp ../g2p_eval/util.hpp ../g2p_eval/utf8.h ../g2p_eval/utf8/checked.h ../g2p_eval/utf8/unchecked.h ../g2p_eval/utf8/core.h main.cpp

AM_LDFLAGS =  -L/usr/local/lib64/fst -L/usr/local/lib/fst -lfst -lfstfar -lfstfarscript -lfstscript -ldl -lngram
