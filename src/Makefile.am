ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CXXFLAGS = $(TAGLIB_CFLAGS) $(FFMPEG_CFLAGS) -Wno-conversion-null -Wno-deprecated

bin_PROGRAMS = audiotag

# using git version instead of the am values in config.h
audiotag_CPPFLAGS = -I$(top_srcdir)/lib
audiotag_CXXFLAGS = $(AM_CXXFLAGS)
audiotag_LDADD = $(TAGLIB_LIBS) $(FFMPEG_LIBS) -lz
audiotag_SOURCES = \
	audiotag.cc \
	File.cc \
	Meta.cc \
	MetaOut.cc \
	Ops.cc

