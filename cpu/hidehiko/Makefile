include ../cpu.inc

all: cpu

cpu: main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) -o $@ -c $(CXXFLAGS) $<

clean:
	rm -f *.o cpu
