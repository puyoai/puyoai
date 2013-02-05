cmake_minimum_required(VERSION 2.8)

cpu_setup("ichikawa")

set(CMAKE_CXX_FLAGS "-Wall")

function(ichikawa_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

ichikawa_add_executable(gtr_main gtr_main.cc)
ichikawa_add_executable(rensabi_main rensabi_main.cc)
ichikawa_add_executable(death_tower_main EXCLUDE_FROM_ALL death_tower_main.cc)
