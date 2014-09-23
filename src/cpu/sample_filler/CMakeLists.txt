cmake_minimum_required(VERSION 2.8)

cpu_setup("sample-filler")

function(sample_filler_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

sample_filler_add_executable(sample_filler main.cc)

cpu_add_runner(run.sh)
cpu_add_runner(run_peak.sh)
cpu_add_runner(run_nohoho.sh)
