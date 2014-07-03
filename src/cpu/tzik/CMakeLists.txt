cmake_minimum_required(VERSION 2.8)

cpu_setup("tzik")

function(tzik_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

tzik_add_executable(tzik main.h main.cc)

cpu_add_runner(sandbox)
add_library(sandbox_inject SHARED sandbox_inject.cc)
