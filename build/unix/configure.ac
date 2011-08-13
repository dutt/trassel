AC_INIT([trassel], [0.1], [micke.modin@gmail.com], [trassel], [https://github.com/dutt/trassel])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_CC_C_O
AC_LANG_CPLUSPLUS

DEFINES=""
OLDCXXFLAGS="$CXXFLAGS"
CXXFLAGS="-Wall $OLDCXXFLAGS"
AC_MSG_CHECKING([if $CC accepts -Wall])
AC_COMPILE_IFELSE(AC_LANG_PROGRAM([],[return 0;]),
                  DEFINES="-Wall"
                  AC_MSG_RESULT([yes]),
                  AC_MSG_RESULT([no]))
CXXFLAGS="-std=c++0x $OLDCXXFLAGS"
AC_MSG_CHECKING([if $CC accepts -std=c++0x])
AC_COMPILE_IFELSE(AC_LANG_PROGRAM([],[return 0;]),
                  DEFINES="-std=c++0x"
                  AC_MSG_RESULT([yes]),
                  AC_MSG_RESULT([no]))
CXXFLAGS="$OLDCXXFLAGS"
AC_SUBST(DEFINES)

AC_ARG_WITH([boost],
            [AS_HELP_STRING([--with-boost=PATH],
                            [use boost in PATH])],
            [], [with_boost=/usr])
AS_IF([test "x$with_boost" = "xyes"],
      AC_MSG_FAILURE([--with-boost was given but without a PATH]))
AS_IF([test "x$with_boost" = "xno"],
      AC_MSG_FAILURE([--without-boost was given but boost is needed]))
BOOST_CXXFLAGS="-I${with_boost}/include"
BOOST_LIBS="-L${with_boost}/lib64 -L${with_boost}/lib32 -L${with_boost}/lib -lboost_thread"
AC_SUBST(BOOST_CXXFLAGS)
AC_SUBST(BOOST_LIBS)

AC_OUTPUT([Makefile])
