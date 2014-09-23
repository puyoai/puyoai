cmake_minimum_required(VERSION 2.8)

cpu_setup("sample-suicide")

function(sample_suicide_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

sample_suicide_add_executable(sample_suicide main.cc)

cpu_add_runner(run.sh)
cpu_add_runner(run_right_turn.sh)
