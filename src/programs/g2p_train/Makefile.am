pkglibexecdir = $(libexecdir)/@PACKAGE@
pkglibexec_PROGRAMS = g2p_train

g2p_train_SOURCES = FstPathFinder.cpp  FstPathFinder.hpp  g2p_train.cpp  M2MFstAligner.cpp  M2MFstAligner.hpp  utils.hpp main.cpp

AM_LDFLAGS =  -L/usr/local/lib64/fst -L/usr/local/lib/fst -lfst -lfstfar -lfstfarscript -lfstscript -ldl -lngram
