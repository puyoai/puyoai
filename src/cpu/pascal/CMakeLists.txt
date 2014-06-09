cmake_minimum_required(VERSION 2.8)

cpu_setup("pascal")

puyoai_add_cxx_flags("-Wall")

add_library(pascal_lib cpu_interface.cc deprecated_field.cc)

function(pascal_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_libraries(${exe} pascal_lib)
  cpu_target_link_common_libraries(${exe})
endfunction()

pascal_add_executable(cpu main.cc)
cpu_add_runner(run.sh)
# TODO(hamaji): Cannot link due to missing field.cc
pascal_add_executable(cpu2 EXCLUDE_FROM_ALL cpu2.cc)
