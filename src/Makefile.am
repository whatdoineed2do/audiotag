ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CXXFLAGS = $(TAGLIB_CFLAGS)
AM_LDFLAGS = $(TAGLIB_LDFLAGS)

bin_PROGRAMS = audiotag

audiotag_CPPFLAGS = -I$(top_srcdir)/lib
audiotag_CXXFLAGS = $(AM_CXXFLAGS)
audiotag_LDFLAGS = $(TAGLIB_LIBS)
audiotag_LDADD =
audiotag_SOURCES = \
	audiotag.cc \
	audiotagfile.cc \
	audiotagfile.h \
	audiotag.h \
	audiotagmeta.cc \
	audiotagmeta.h \
	audiotagmetaout.cc \
	audiotagmetaout.h \
	audiotagops.h

