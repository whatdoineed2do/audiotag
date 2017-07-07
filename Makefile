all:	objs audiotag
clean:
	@rm -f *.o audiotag core a.out

CXXFLAGS += -gdwarf-2 -DNDEBUG
CXXFLAGS = -g -I/usr/include/taglib -I/home/ray/tools/include/
LDFLAGS = -g -L/home/ray/tools/lib -ltag -lz

%.o     : %.cc  ;       $(CXX) -c $(CXXFLAGS) $<


objs:	audiotagmeta.o audiotagmetaout.o audiotagfile.o audiotag.o

audiotag:	audiotag.o audiotagmeta.o audiotagfile.o audiotagmetaout.o
	g++ $^ $(shell taglib-config --libs) $(LDFLAGS) -o $@ 
	cp -f .backup/test*{flac,m4a,mp3} .


audiotag.o:	audiotag.cc audiotagmeta.h audiotagfile.h audiotagmetaout.h
audiotagmeta.o: audiotagmeta.cc audiotagmeta.h audiotagfile.h audiotagmetaout.h
audiotagmetaout.o: audiotagmetaout.cc audiotagmetaout.h audiotagmeta.h
audiotagfile.o: audiotagfile.cc audiotagmeta.h audiotagfile.h
