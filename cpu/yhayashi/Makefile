include ../cpu.inc

CXXFLAGS+=-O -g -W -Wall -Werror
CXXFLAGS+=-I/opt/local/include -L/opt/local/lib

SRCS=main.cc


all: main

main: main.cc ../../core/field.cc ../../core/ctrl.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
