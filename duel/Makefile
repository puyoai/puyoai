USE_SDL=0

CXXFLAGS+=-O2 -I.. -g -MMD
# Comment out if you have a real glib library.
ifneq ($(HAS_GLOG), 1)
  CXXFLAGS+=-I../third_party -L../third_party
else
  CXXFLAGS+=`pkg-config --cflags --libs libgflags libglog`
endif
LIBS+=-L../core -lpuyocore

SRCS=$(filter-out duel.cc %linux.cc %windows.cc %test.cc screen.cc commentator.cc, $(wildcard *.cc))
OBJS=$(subst .c,.o,$(subst .cc,.o,$(SRCS)))

ifeq ($(USE_SDL),1)
  CXXFLAGS+=-DUSE_SDL `sdl-config --cflags`
  CFLAGS+=-DUSE_SDL `sdl-config --cflags`
  SRCS+=SDL_kanji.c
  SRCS+=screen.cc commentator.cc
  LIBS+=`sdl-config --libs` -lSDL_ttf
  TEST_BINARIES+=commentator_test
endif

ifeq ($(MSYSTEM), MINGW32)
  BINARIES=windows.exe
else
  BINARIES=duel
endif

all: $(BINARIES)

duel: $(OBJS) duel.o connector_manager_linux.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lglog $(LIBS)

windows.exe: $(OBJS) duel.o connector_manager_windows.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lglog -mwindows $(LIBS)

################################################################################
# Test section.
################################################################################
HAS_GTEST=$(shell ld -lgtest && echo "Success")
TEST_BINARIES+=field_realtime_test
LDFLAGS=-lgtest_main

test: $(TEST_BINARIES)

CHECK_TEST_BINARIES=$(TEST_BINARIES:%=check_%)
check: $(CHECK_TEST_BINARIES)

check_%: %
	./$<

tests:
	$(if $(HAS_GTEST), make tests_main)

tests_main: $(TEST_BINARIES)
	./field_realtime_test

field_realtime_test: $(OBJS) field_realtime_test.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lglog $(LIBS) -lgtest

commentator_test: commentator_test.o $(OBJS) commentator.o screen.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) $(LDFLAGS) -lgtest

clean:
	rm -f *~ *.o *.d $(BINARIES) $(TEST_BINARIES)

-include *.d
