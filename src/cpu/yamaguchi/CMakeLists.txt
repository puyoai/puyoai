cmake_minimum_required(VERSION 2.8)

cpu_setup("yamaguchi")

function(yamaguchi_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

yamaguchi_add_executable(yamaguchi main.cc)

cpu_add_runner(run.sh)

