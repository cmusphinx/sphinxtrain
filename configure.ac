# Automake-based build system

AC_INIT(SphinxTrain, 5prealpha)
AC_CONFIG_HEADERS([include/config.h include/sphinx_config.h])
AM_INIT_AUTOMAKE([no-define foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CXX

LT_INIT

AC_HEADER_STDC
AC_C_BIGENDIAN
AC_TYPE_SIGNAL
AC_CHECK_TYPES(long long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(long)
AC_CHECK_FUNCS([popen perror snprintf])
AC_CHECK_HEADER(errno.h)

dnl
dnl Check for Lapack stuff unless disabled
dnl
use_lapack=true
use_internal_lapack=false
AC_ARG_WITH(lapack,
	AS_HELP_STRING([--without-lapack],
		       [Disable matrix algebra support (depends on LAPACK)]),[
if test x$withval = xno; then
   use_lapack=false
fi
])
if test x$use_lapack = xtrue; then
   AC_CHECK_LIB(lapack, sgesv_,,use_internal_lapack=true)
   if test x$use_internal_lapack != xtrue; then
      AC_CHECK_LIB(blas, sgemm_,,use_internal_lapack=true)
   fi
   AC_DEFINE(WITH_LAPACK, [], [Enable matrix algebra with LAPACK])
fi
AM_CONDITIONAL(LAPACK_LITE, test x$use_internal_lapack = xtrue)
AM_CONDITIONAL(USE_LAPACK, test x$use_lapack = xtrue)

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
src/libs/libsphinxbase/Makefile
src/libs/libsphinxbase/fe/Makefile
src/libs/libsphinxbase/feat/Makefile
src/libs/libsphinxbase/lm/Makefile
src/libs/libsphinxbase/util/Makefile
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
src/programs/sphinx_fe/Makefile
src/programs/sphinx_cepview/Makefile
src/programs/tiestate/Makefile
test/Makefile
])

AC_OUTPUT
