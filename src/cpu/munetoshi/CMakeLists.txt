cmake_minimum_required(VERSION 2.8)

cpu_setup("munetoshi")

function(munetoshi_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

munetoshi_add_executable(minim main.cc ai.cc utilities.cc evaluator.cc)
munetoshi_add_executable(minim_eval eval.cc ai.cc utilities.cc evaluator.cc)

cpu_add_runner(run.sh)
