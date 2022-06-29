SUBDIRS = util fe feat lm

noinst_LTLIBRARIES = libsphinxbase.la

libsphinxbase_la_SOURCES =
libsphinxbase_la_LIBADD = $(LTLIBICONV) \
	util/libsphinxutil.la \
	fe/libsphinxfe.la \
	feat/libsphinxfeat.la \
	lm/libsphinxlm.la
