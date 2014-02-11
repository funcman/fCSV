CXXFLAGS?=-Os -Wall

all: libfcsv.a

libfcsv.a: fcsv.o
	$(AR) -r libfcsv.a fcsv.o

fcsv.o: fcsv.cpp fcsv.h
	$(CXX) $(CXXFLAGS) -c fcsv.cpp

clean:
	$(RM) fcsv.o libfcsv.a

install: all
	cp fcsv.h /usr/include
	cp libfcsv.a /usr/lib

uninstall:
	$(RM) /usr/include/fcsv.h
	$(RM) /usr/lib/libfcsv.a
