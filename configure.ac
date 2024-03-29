# autoreconf --install
# autoconf
#

AC_PREREQ([2.69])
AC_INIT([audiotag],[m4_esyscmd([git describe --tags --dirty --always | tr -d '\n'])],[https://github.com/whatdoineed2do/audiotag/issues],[https://github.com/whatdoineed2do/audiotag],[whatdoineed2do@nospam-github.com])
AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip])
AC_CONFIG_MACRO_DIRS([m4])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

AX_CXX_COMPILE_STDCXX([14], [noext], [mandatory])



PKG_CHECK_MODULES(TAGLIB, [taglib >= 1.11.1])
AC_ARG_ENABLE([ffmpeg],
                [AS_HELP_STRING([--enable-ffmpeg], [Use ffmpeg for more accurate input assessment and audio hash identification])],
                [want_ffmpeg=$enableval],
		[want_ffmpeg=yes]
	     )

if test x"$want_ffmpeg" = x"yes"; then
    PKG_CHECK_MODULES([FFMPEG], [libavformat libavcodec libavutil], [have_ffmpeg=yes], [have_ffmpeg=no])
    if test x"$have_ffmpeg" = x"yes"; then
	AH_TEMPLATE([HAVE_FFMPEG], [Whether ffmpeg is installed, used for more accurate input file assessment])
	AC_DEFINE_UNQUOTED(HAVE_FFMPEG, 1)
    fi
fi

AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LIBS)
AM_CONDITIONAL(HAVE_FFMPEG, test x"$have_ffmpeg" = x"yes")

AC_ARG_WITH([json-c], [AS_HELP_STRING([--with-json-c], [Do not build json-c out module])], [], [with_json_c=no])

AS_IF([test x"$with_json_c" != x"no"],
        [PKG_CHECK_MODULES([JSONC], [json-c], [], [AC_MSG_ERROR(json-c not found)])]
	[AC_DEFINE([HAVE_JSONC], [1], [Provde json-c as output module])]
     )
AC_SUBST(JSONC_CFLAGS)
AC_SUBST(JSONC_LIBS)
AM_CONDITIONAL([HAVE_JSONC], [test x"$with_json_c" != x"no"])


AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],
	[enable debug (default is no)])],
	[no_debug=$enableval],
	[no_debug=true])
if test "$no_debug" = true; then
        building_debug=no
        AC_DEFINE(NDEBUG,1,[debug code])
else
        building_debug=yes
        AC_DEFINE(DEBUG,1,[debug code])
fi

AC_CHECK_FUNCS([utimes])
AC_MSG_CHECKING([for stat.st_mtim])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/stat.h>]],
[[
  struct stat st;
  memset(&st.st_mtim, 0, sizeof(st.st_mtim));
  return 0;
]])],
        [
          AC_MSG_RESULT([yes])
          AC_DEFINE_UNQUOTED([HAVE_ST_MTIM], 1, [Define to 1 if have struct stat.st_mtim.])
        ],
        [
          AC_MSG_RESULT([no])
          AC_DEFINE_UNQUOTED([HAVE_ST_MTIM], 0, [Define to 1 if have struct stat.st_mtim.])
        ]
     )


AC_CONFIG_FILES([
Makefile
src/Makefile
tests/Makefile
])
AC_OUTPUT
