# Automake-based build system

AC_INIT(SphinxTrain, 5prealpha)
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE([no-define foreign])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_C_BIGENDIAN

LT_INIT

AC_CHECK_LIB(m, log)
AC_CHECK_LIB(g2c, d_log)

AC_PATH_PROGS(PERL, [perl5 perl])
if test "x$PERL" = x; then
	cat <<\EOF
*** Could not find 'perl' or 'perl5' on your system.
*** If you know the location of your Perl 5 binary, please set the
*** PERL environment variable to it and re-run ./configure.
*** For example:

$ env PERL=/path/to/your/perl5 ./configure
EOF
	exit 1
fi

dnl Use PIC on x86_64, as we can't build Perl/Python modules otherwise
dnl (and it isn't much slower unlike on register-starved i386)
case $host_cpu in
     x86_64*)
	CFLAGS="$CFLAGS -fPIC -DPIC"
	;;
esac

dnl
dnl Get SphinxBase source from command line if given
dnl
AC_ARG_WITH(sphinxbase,
	AS_HELP_STRING([--with-sphinxbase=DIRECTORY],
			[Look for SphinxBase source files in DIRECTORY.  If this is 'auto',
			 the system-wide installation will be used.]),
			sphinxbase=$withval)

dnl
dnl Get SphinxBase build directory from command line if given
dnl
AC_ARG_WITH(sphinxbase-build,
	AS_HELP_STRING([--with-sphinxbase-build=DIRECTORY],
			[Look for SphinxBase object files in DIRECTORY]),
	sphinxbasebuild=$withval)


dnl
dnl Check for SphinxBase in parent directories
dnl
if test x$sphinxbase = x; then
   dn=`dirname $0`
   case "$dn" in
	.)
	sbdir="`pwd`/.."
	;;
	[\\/]* | ?:[\\/]*)
	sbdir="$dn/.."
	;;
	*)
	sbdir="`pwd`/$dn/.."
	;;
   esac
   # Look for sphinxbase in the parent directory
   for sb in "$sbdir/sphinxbase" \
	  "$sbdir/sphinxbase"*; do
       AC_MSG_CHECKING([for sphinxbase in $sb])
       if test -f "$sb/include/sphinxbase/cmd_ln.h"; then
          sphinxbase="$sb"
	  AC_MSG_RESULT(yes)
	  break
       else
	  AC_MSG_RESULT(no)
       fi
   done
fi

dnl
dnl Check for system SphinxBase if none was passed to us
dnl Also allow --with-sphinxbase=auto to use system one explicitly
dnl
if test x$sphinxbase = x || test x$sphinxbase = xauto; then
   sphinxbase=
   if test "x$HAVE_PKGCONFIG" = "xno"; then
      SphinxBase_CFLAGS = "-Iusr/include/sphinxbase -I/usr/local/include/sphinxbase"
      SphinxBase_LIBS = "-lsphinxbase"
   else
      PKG_CHECK_MODULES(SphinxBase, [sphinxbase],,[
      AC_MSG_FAILURE(dnl
[SphinxBase was not found in the parent directories or
installed system-wide on your system.  Make sure that
you have compiled it or installed it and that the
PKG_CONFIG_PATH environment variable is set correctly, if
it was installed in a non-standard prefix.])])
   fi
   LIBS="$LIBS $SphinxBase_LIBS"
   CPPFLAGS="$CPPFLAGS $SphinxBase_CFLAGS"
   AC_CHECK_HEADER(sphinx_config.h,,[
   AC_MSG_FAILURE(dnl
[SphinxBase was not found on your system.])])
fi

dnl
dnl Now verify SphinxBase if defined
dnl Sadly, this doesn't work when cross-compiling (for some dumb reason...)
dnl
: ${sphinxbasebuild=$sphinxbase}
if test x$sphinxbase != x && test x$cross_compiling = xyes; then
   CPPFLAGS="-I$sphinxbase/include -I$sphinxbasebuild/include $CPPFLAGS"
   LIBS="$LIBS -lsphinxbase"
   # We include libsphinxbase/.libs because of stupid broken versions of libtool
   LDFLAGS="$LDFLAGS -L$sphinxbasebuild/src/libsphinxad \
		     -L$sphinxbasebuild/src/libsphinxbase \
		     -L$sphinxbasebuild/src/libsphinxbase/.libs"
fi
if test x$sphinxbase != x && test x$cross_compiling != xyes; then
   AC_CHECK_FILE($sphinxbase/include/sphinxbase/prim_type.h,CPPFLAGS="-I$sphinxbase/include $CPPFLAGS",
   AC_ERROR(
[SphinxBase headers not found in $sphinxbase.  Please use the
--with-sphinxbase option to `configure' to specify the location of
SphinxBase.  Run $0 --help for more information.]))
fi
# We include libsphinxbase/.libs because of stupid broken versions of libtool
if test x$sphinxbasebuild != x && test x$cross_compiling != xyes; then
   AC_CHECK_FILE($sphinxbasebuild/src/libsphinxbase/libsphinxbase.la,
[CPPFLAGS="-I$sphinxbasebuild/include $CPPFLAGS"
LIBS="$LIBS -lsphinxbase"
LDFLAGS="$LDFLAGS -L$sphinxbasebuild/src/libsphinxad \
		  -L$sphinxbasebuild/src/libsphinxbase \
		  -L$sphinxbasebuild/src/libsphinxbase/.libs"],
		   AC_ERROR(
[SphinxBase libraries were not found in $sphinxbasebuild.
Use the --with-sphinxbase-build option to `configure' to specify
the build directory for SphinxBase.  Run $0 --help for more information.]))
fi

AC_ARG_ENABLE(g2p-decoder,
 [  --enable-g2p-decoder  Enable the g2p decoder],
 [ g2p_decoder=yes ])
AM_CONDITIONAL([G2P_DECODER], [test "x$g2p_decoder" = "xyes"])


dnl ###############################
dnl
dnl G2P related stuff start here
dnl
dnl ###############################
if   test "x$g2p_decoder" = "xyes" ; then

AC_LANG([C++])

# Get openfst and opengrm library and include locations
AC_ARG_WITH([openfst-includes],
  [AS_HELP_STRING([--with-openfst-includes],
      [location of the openfst headers, defaults to /usr/local/include])],
        [OPENFST_CFLAGS="-I$withval"],
        [OPENFST_CFLAGS="-I/usr/local/include"])
AC_SUBST([OPENFST_CFLAGS])
          
AC_ARG_WITH([openfst-libs],
            [AS_HELP_STRING([--with-openfst-libs], [location of the openfst libraries])],
            [OPENFST_LIBS="-L$withval -lfst -lfstfar -lfstfarscript -lfstscript -ldl -lngram"],
            [OPENFST_LIBS="-lfst -lfstfar -lfstfarscript -lfstscript -ldl -lngram"])
AC_SUBST([OPENFST_LIBS])
	
# Check for openFst headers
saved_cppflags="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${OPENFST_CFLAGS}"
AC_CHECK_HEADER([fst/fstlib.h], [],
 [AC_MSG_ERROR([fst/fstlib.h header not found])]
 []
)	
AC_CHECK_HEADER([ngram/ngram-input.h], [],
 [AC_MSG_ERROR([ngram/ngram-input.h header not found])]
)	
CPPFLAGS="${saved_cppflags}"

fi

AC_SUBST(sphinxbase)
AC_SUBST(sphinxbasebuild)

AC_CONFIG_FILES([
Makefile
etc/Makefile
include/Makefile
python/Makefile
scripts/Makefile
src/Makefile
src/programs/Makefile
src/libs/Makefile
src/libs/libclust/Makefile
src/libs/libcommon/Makefile
src/libs/libio/Makefile
src/libs/libmllr/Makefile
src/libs/libmodinv/Makefile
src/programs/agg_seg/Makefile
src/programs/bldtree/Makefile
src/programs/bw/Makefile
src/programs/cdcn_norm/Makefile
src/programs/cdcn_train/Makefile
src/programs/cp_parm/Makefile
src/programs/delint/Makefile
src/programs/inc_comp/Makefile
src/programs/init_gau/Makefile
src/programs/init_mixw/Makefile
src/programs/g2p_eval/Makefile
src/programs/g2p_train/Makefile
src/programs/kdtree/Makefile
src/programs/kmeans_init/Makefile
src/programs/make_quests/Makefile
src/programs/map_adapt/Makefile
src/programs/mixw_interp/Makefile
src/programs/mk_flat/Makefile
src/programs/mk_mdef_gen/Makefile
src/programs/mk_mllr_class/Makefile
src/programs/mk_s2sendump/Makefile
src/programs/mk_ts2cb/Makefile
src/programs/mllr_solve/Makefile
src/programs/mllr_transform/Makefile
src/programs/norm/Makefile
src/programs/param_cnt/Makefile
src/programs/printp/Makefile
src/programs/prunetree/Makefile
src/programs/tiestate/Makefile
test/Makefile
])

AC_OUTPUT
