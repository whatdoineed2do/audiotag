all:	objs audiotag data
clean:
	@rm -f *.o audiotag core a.out

CXXFLAGS += -gdwarf-2 -DNDEBUG

CXXFLAGS = -g $(shell pkg-config taglib --cflags)
LDFLAGS = -g $(shell pkg-config taglib --libs) -lz

%.o     : %.cc  ;       $(CXX) -c $(CXXFLAGS) $<


objs:	audiotagmeta.o audiotagmetaout.o audiotagfile.o audiotag.o 

data:
	git checkout f1856a4 test.mp3 test-ntags.mp3 test.flac test.m4a  test-pic.flac test-pic.m4a  test-pic.mp3 test-alltags.mp3

audiotag:	audiotag.o audiotagmeta.o audiotagfile.o audiotagmetaout.o
	g++ $^ $(LDFLAGS) -o $@ 


audiotag.o:		audiotag.cc audiotag.h audiotagfile.h audiotagmeta.h audiotagmetaout.h audiotagops.h
audiotagfile.o:		audiotagfile.cc audiotagfile.h audiotagmeta.h
audiotagmeta.o:		audiotagmeta.cc audiotagmeta.h audiotag.h audiotagfile.h audiotagmetaout.h
audiotagmetaout.o:	audiotagmetaout.cc audiotagmetaout.h audiotagmeta.h

test:	test.cc
	g++ $^ $(LDFLAGS) -o $@ 
