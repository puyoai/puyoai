cmake_minimum_required(VERSION 2.8)

cpu_setup("hiroshimizuno")

function(hiroshimizuno_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

hiroshimizuno_add_executable(hikuitokoro hikuitokoro.cc)

# TODO(hamaji): Followings cannot be built by clang.
puyoai_add_cxx_flags("-march=native -masm=intel")
hiroshimizuno_add_executable(f EXCLUDE_FROM_ALL f.cc)
hiroshimizuno_add_executable(t EXCLUDE_FROM_ALL test.cc sse.cc)

# TODO(hamaji): Following targets don't have corresponding files.
#
# maton: maton.cc frame.h linux.h
# 	g++ -O3 -march=native maton.cc -o maton
# 
# clock: clock.cc
# 	g++ -O3 -march=native -masm=intel clock.cc -o clock
# 	./clock
# 	rm clock
# 
# p: p.cc
# 	g++ -O3 -march=native -o p p.cc ../../core/field.cc ../../core/ctrl.cc -lglog
# 	./p
# 	rm p
# 
# a: asm.cc
# 	g++ -O0 -march=native -masm=intel -S -o asm.s asm.cc
# 	cat asm.s
# 	rm asm.s
