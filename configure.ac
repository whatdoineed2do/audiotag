# autoreconf --install
# autoconf
#

AC_PREREQ(2.64)
AC_INIT([audiotag], [1.2.4], [https://github.com/whatdoineed2do/audiotag/issues], [https://github.com/whatdoineed2do/audiotag], [whatdoineed2do@nospam-github.com])
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
#PKG_CHECK_MODULES(JSONC, [json-c])

AC_ARG_ENABLE([debug-code],
	[AS_HELP_STRING([--enable-debug],
	[enable debug (default is no)])],
	[no_debug=false],
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


AC_OUTPUT([
Makefile
src/Makefile
tests/Makefile
])
