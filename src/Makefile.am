ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CXXFLAGS = $(TAGLIB_CFLAGS) $(FFMPEG_CFLAGS) -Wall -Wno-reorder -Wno-unused-but-set-variable -Wno-conversion-null -Wno-deprecated

bin_PROGRAMS = audiotag

# using git version instead of the am values in config.h
audiotag_CPPFLAGS = -I$(top_srcdir)/lib
audiotag_CXXFLAGS = $(AM_CXXFLAGS)
audiotag_LDADD = $(TAGLIB_LIBS) $(FFMPEG_LIBS) -lz
audiotag_SOURCES = \
	audiotag.cc \
	File.cc \
	Meta.cc \
	MetaMP3.cc MetaFlac.cc MetaM4a.cc \
	MetaOut.cc \
	Ops.cc

if HAVE_JSONC
audiotag_LDADD += $(JSONC_LIBS)
endif
