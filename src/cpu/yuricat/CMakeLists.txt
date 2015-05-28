cmake_minimum_required(VERSION 2.8)

cpu_setup("yuricat")

function(yuricat_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

yuricat_add_executable(yuricat main.cc)

cpu_add_runner(run.sh)

