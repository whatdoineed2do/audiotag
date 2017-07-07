all:	objs audiotag data
clean:
	@rm -f *.o audiotag core a.out

CXXFLAGS += -gdwarf-2 -DNDEBUG
CXXFLAGS = -g -I/usr/include/taglib -I/home/ray/tools/include/
LDFLAGS = -g -L/home/ray/tools/lib -ltag -lz

%.o     : %.cc  ;       $(CXX) -c $(CXXFLAGS) $<


objs:	audiotagmeta.o audiotagmetaout.o audiotagfile.o audiotag.o 

data:
	git checkout cfe5718 test.mp3 test-ntags.mp3 test.flac test.m4a  test-pic.flac  test-pic.mp3

audiotag:	audiotag.o audiotagmeta.o audiotagfile.o audiotagmetaout.o
	g++ $^ $(shell taglib-config --libs) $(LDFLAGS) -o $@ 
#cp -f .backup/test*{flac,m4a,mp3} .


audiotag.o:	audiotag.cc audiotagmeta.h audiotagfile.h audiotagmetaout.h audiotagops.h
audiotagmeta.o: audiotagmeta.cc audiotagmeta.h audiotagfile.h audiotagmetaout.h
audiotagmetaout.o: audiotagmetaout.cc audiotagmetaout.h audiotagmeta.h
audiotagfile.o: audiotagfile.cc audiotagmeta.h audiotagfile.h
