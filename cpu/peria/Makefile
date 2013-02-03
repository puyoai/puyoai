SRC=game.cc field.cc decision.cc
TESTS=field_test

include ../cpu.inc

PERF=-pg
CXXFLAGS +=-O -g $(PERF) -Wall -Wextra -Werror -MMD
LDFLAGS  +=-lpthread $(PERF)

OBJ=$(SRC:.cc=.o)

all: peria

peria: main.o $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

%_test: %_test.o $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) -lgtest -lgtest_main
	./$@

test: $(TESTS)

clean:
	rm -f *~ *.o *.d peria $(TESTS)
