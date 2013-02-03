include ../cpu.inc

hikuitokoro: hikuitokoro.cc ../../core/field.cc ../../core/ctrl.cc
	g++ $(CXXFLAGS) hikuitokoro.cc ../../core/field.cc ../../core/ctrl.cc -o hikuitokoro $(LDFLAGS)

clean:
	rm -f *~ *.o hikuitokoro

maton: maton.cc frame.h linux.h
	g++ -O3 -march=native maton.cc -o maton

f: f.cc
	g++ -O3 -march=native -masm=intel f.cc -o test
	./test
	rm test

sse.o: sse.cc sse.h
	g++ -O3 -march=native -masm=intel sse.cc -c -o sse.o

clock: clock.cc
	g++ -O3 -march=native -masm=intel clock.cc -o clock
	./clock
	rm clock

p: p.cc
	g++ -O3 -march=native -o p p.cc ../../core/field.cc ../../core/ctrl.cc -lglog
	./p
	rm p

a: asm.cc
	g++ -O0 -march=native -masm=intel -S -o asm.s asm.cc
	cat asm.s
	rm asm.s

t: test.cc sse.o
	g++ -O3 -march=native -masm=intel test.cc sse.o -o t
	./t
	rm t
