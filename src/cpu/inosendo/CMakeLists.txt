cmake_minimum_required(VERSION 2.8)

cpu_setup("inosendo")

add_library(inosendo_lib
            gazer.cc
            rensa_hand_tree.cc)

function(inosendo_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_libraries(${exe} inosendo_lib)
  cpu_target_link_common_libraries(${exe})
endfunction()

cpu_add_runner(run.sh)

inosendo_add_executable(inosendo main.cc)

