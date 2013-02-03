include ../cpu.inc

CXXFLAGS+=-O3 -Wall -I../../ -MMD -g
LDFLAGS+=-L../../cpu -L../../core 
LIBS+=-lpuyocpu -lpuyocore

BINARIES=cpu #cpu2
all: $(BINARIES)

cpu: main.cc ctrl.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

cpu2: cpu2.cc ctrl.o field.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

clean:
	rm -f *~ *.o $(BINARIES)
