include ../cpu.inc

CXXFLAGS+= -O2 -g -Wall -Wextra -Werror -MMD

SRCS = ai.cc ai_manager.cc ctrl.cc field.cc field_evaluator.cc game.cc player_info.cc protocol.cc puyo_possibility.cc
OBJS = $(subst .cc,.o,$(SRCS))

TSRCS = $(wildcard *_test.cc)
TESTS = $(TSRCS:.cc=)
TEST_RESULTS = $(TESTS:=.out)

all: shinyak_cpu

cpu: shinyak_cpu

%.out: %
	./$< | tee $@

shinyak_cpu: $(OBJS) main.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

interactive: $(OBJS) interactive.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

%_test: %_test.o $(OBJS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LDFLAGS) -lgtest -lgtest_main

performance_test: $(OBJS) performance_test.o ../../util/tsc.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lgtest -lgtest_main

solver: $(OBJS) solver.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

check: $(TEST_RESULTS)

test: $(TESTS)

clean:
	rm -f *~ *.d *.o shinyak_cpu interactive solver $(TEST_RESULTS) $(TESTS)

-include *.d
