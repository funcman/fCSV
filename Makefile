CXXFLAGS?=-Os -Wall

all: libfcsv.a

libfcsv.a: fcsv.o fstr.o
	$(AR) -r libfcsv.a fcsv.o fstr.o

fcsv.o: fcsv.cpp fcsv.h
	$(CXX) $(CXXFLAGS) -c fcsv.cpp

fstr.o: fStr/fstr.c fStr/fstr.h
	$(CC) $(CXXFLAGS) -c fStr/fstr.c

fStr/fstr.c:
	git submodule update --init

fStr/fstr.h:
	git submodule update --init

clean:
	$(RM) fcsv.o fstr.o libfcsv.a

install: all
	cp fcsv.h /usr/include
	cp libfcsv.a /usr/lib

uninstall:
	$(RM) /usr/include/fcsv.h
	$(RM) /usr/lib/libfcsv.a
