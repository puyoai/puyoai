include ../cpu.inc

CXXFLAGS+=-I../../ -I./ -MMD -g
LDFLAGS+=-L../ -L../../core
LIBS=-lpuyocpu -lpuyocore

sample: sample.cc
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f *~ *.o sample
