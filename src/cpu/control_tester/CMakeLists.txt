cmake_minimum_required(VERSION 2.8)

cpu_setup("control_tester")

function(control_tester_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

control_tester_add_executable(control_tester main.cc)

cpu_add_runner(run.sh)
cpu_add_runner(run_peak.sh)
cpu_add_runner(run_nohoho.sh)
cpu_add_runner(run_quickturn.sh)
cpu_add_runner(run_right.sh)
