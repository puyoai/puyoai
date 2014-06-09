cmake_minimum_required(VERSION 2.8)

cpu_setup("ichikawa")

puyoai_add_cxx_flags("-Wall")

function(ichikawa_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_common_libraries(${exe})
endfunction()

ichikawa_add_executable(gtr_main gtr_main.cc deprecated_field.cc)
ichikawa_add_executable(rensabi_main rensabi_main.cc deprecated_field.cc)
ichikawa_add_executable(death_tower_main EXCLUDE_FROM_ALL death_tower_main.cc)

cpu_add_runner(run.sh)
cpu_add_runner(rensabi.sh)
cpu_add_runner(run-slow.sh)
cpu_add_runner(death_tower.sh)
