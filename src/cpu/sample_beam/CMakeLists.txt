cmake_minimum_required(VERSION 2.8)

cpu_setup("sample_beam")

function(sample_beam_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

sample_beam_add_executable(beam_search_ai beam_search_ai.cc main.cc)

cpu_add_runner(run_full.sh)
cpu_add_runner(run_2dub.sh)
