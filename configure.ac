# autoreconf --install
# autoconf
#

AC_PREREQ(2.64)
AC_INIT([audiotag], [1.2.1], [https://github.com/whatdoineed2do/audiotag], [whatdoineed2do@nospam-github.com])
AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(TAGLIB, [taglib >= 1.11.1])

#-gdwarf-2 -DNDEBUG
AC_ARG_ENABLE([debug-code],
	[AS_HELP_STRING([--enable-debug],
	[enable debug (default is no)])],
	[no_debug=false],
	[no_debug=true])
if test "$no_debug" = true; then
        building_debug=no
        AC_DEFINE(STRIP_DEBUG_CODE,1,[Strip debug code])
else
        building_debug_code=yes
fi


AC_OUTPUT([
Makefile
src/Makefile
])
