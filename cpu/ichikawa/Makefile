include ../cpu.inc

all: cpu
cpu: gtr_main rensabi_main

gtr_main: gtr_main.cc ../../core/field.cc ../../core/field.h ../../core/ctrl.cc ../../core/ctrl.h
	g++ $(CXXFLAGS) -Wall -o $@ gtr_main.cc ../../core/field.cc ../../core/ctrl.cc $(LDFLAGS) -pthread

rensabi_main: rensabi_main.cc ../../core/field.cc ../../core/field.h ../../core/ctrl.cc ../../core/ctrl.h
	g++ $(CXXFLAGS) -Wall -o $@ rensabi_main.cc ../../core/field.cc ../../core/ctrl.cc $(LDFLAGS) -pthread

death_tower_main: death_tower_main.cc ../../core/field.cc ../../core/field.h ../../core/ctrl.cc ../../core/ctrl.h
	g++ $(CXXFLAGS) -Wall -o $@ death_tower_main.cc ../../core/field.cc ../../core/ctrl.cc $(LDFLAGS) -pthread

clean:
	rm -f *~ *.o gtr_main rensabi_main death_tower_main
