PROFILE=
CXXFLAGS+=-O -g -W -Wall -Werror -MMD -Wno-deprecated-declarations $(PROFILE) `sdl-config --cflags` `pkg-config --cflags libavformat libswscale` -I..
LDFLAGS+=`sdl-config --libs` -lSDL_image -lSDL_ttf `pkg-config --libs libavformat libswscale libavutil libavcodec` -L/usr/local/lib -lgflags -lglog

ifeq ($(shell uname), Linux)
	LDFLAGS+=-lv4l2
endif

EXES=parse_bmp parse_movie parse_viddev parse_imgs connect capture_test
SRCS=capture.cc movie.cc viddev.cc images.cc source.cc
OBJS=$(SRCS:.cc=.o)

CORE_OBJS=../core/field.o ../core/ctrl.o ../core/plan.o ../util/tsc.o ../duel/connector.o ../duel/connector_manager_linux.o ../duel/screen.o ../duel/commentator.o

all: $(EXES)

parse_bmp: parse_bmp.o $(OBJS) $(CORE_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

parse_movie: parse_movie.o $(OBJS) $(CORE_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

parse_viddev: parse_viddev.o $(OBJS) $(CORE_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

parse_imgs: parse_imgs.o $(OBJS) $(CORE_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

connect: connect.o $(OBJS) $(CORE_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

capture_test: capture_test.o $(OBJS) $(CORE_OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) -lgtest

clean:
	rm -f *.d *.o $(EXES)

-include *.d
