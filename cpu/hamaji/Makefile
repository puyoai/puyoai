#ifeq ($(shell uname),Darwin)
#CXX=clang++
#endif

include ../cpu.inc

PROFILE=
CXXFLAGS+=-O -g -W -Wall -Werror -MMD -Wno-deprecated-declarations $(PROFILE)
LDFLAGS+=-lpthread $(PROFILE)

SRCS=core.cc eval.cc util.cc game.cc field.cc ctrl.cc solo.cc mt19937int.cc rater.cc ratingstats.cc db.cc
OBJS=$(SRCS:.cc=.o)

TSRCS=$(wildcard *_test.cc)
TESTS=$(TSRCS:.cc=)
TEST_RESULTS=$(TESTS:=.out)

CORE_OBJS=../../core/field.o ../../core/ctrl.o ../../util/tsc.o

all: lps study mkdb matchstat

cpu: lps

check: $(TEST_RESULTS)

test: $(TESTS)

%.out: %
	./$< | tee $@

profile:
	$(MAKE) PROFILE=-pg

lps: main.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CORE_OBJS)

study: study.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CORE_OBJS)

mkdb: mkdb.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CORE_OBJS)

matchstat: matchstat.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(CORE_OBJS)

%_test: %_test.o $(OBJS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LDFLAGS) $(CORE_OBJS) -lgtest -lgtest_main

%.o: %.cc
	$(CXX) -o $@ -c $(CXXFLAGS) $<

parse_movie: $(OBJS) $(CORE_OBJS) parse_movie.cc
	$(CXX) -o $@ $^ $(LDFLAGS) -I/usr/include/ffmpeg -lavutil -lavformat -lavcodec -lswscale `sdl-config --cflags --libs` -g -O2

clean:
	rm -f *.d lps *.o study

-include *.d
