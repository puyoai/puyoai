all: tsc.o

tsc.o: tsc.cc tsc.h
	g++ $(CXXFLAGS) -Wall -o $@ -c tsc.cc

clean:
	rm -f *.o *~
