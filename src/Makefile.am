ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CXXFLAGS = $(TAGLIB_CFLAGS) $(JSONC_CFLAGS) -Wno-conversion-null -Wno-deprecated

bin_PROGRAMS = audiotag

# using git version instead of the am values in config.h
audiotag_CPPFLAGS = -I$(top_srcdir)/lib
audiotag_CXXFLAGS = $(AM_CXXFLAGS)
audiotag_LDFLAGS = $(TAGLIB_LIBS) $(JSONC_LIBS)
audiotag_LDADD =
audiotag_SOURCES = \
	audiotag.cc \
	audiotagfile.cc \
	audiotagmeta.cc \
	audiotagmetaout.cc \
	audiotagops.cc

