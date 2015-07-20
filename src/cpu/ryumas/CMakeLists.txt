cmake_minimum_required(VERSION 2.8)

cpu_setup("ryumas")


function(ryumas_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

ryumas_add_executable(ryumas rensa5.cc)

cpu_add_runner(run.sh)