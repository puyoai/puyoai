cmake_minimum_required(VERSION 2.8)

cpu_setup("sample")

function(sample_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

sample_add_executable(sample main.cc)

