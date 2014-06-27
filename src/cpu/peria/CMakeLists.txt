cmake_minimum_required(VERSION 2.8)

cpu_setup("peria")

puyoai_add_cxx_flags("-Wall -Wextra")
puyoai_add_cxx_flags("-Wno-error=unused-parameter -Wno-error=unused-variable")

add_library(peria_lib ai.cc)

function(peria_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_libraries(${exe} peria_lib)
  cpu_target_link_common_libraries(${exe})
endfunction()

function(peria_add_test exe)
  peria_add_executable(${exe} ${exe}.cc)
  cpu_target_link_libraries(${exe} gtest gtest_main)
  if(NOT ARGV1)
    cpu_add_test(${exe})
  endif()
endfunction()

peria_add_executable(peria main.cc)

cpu_add_runner(run.sh)

include_directories(${gtest_SOURCE_DIR}/include
                    ${gtest_SOURCE_DIR})
# TODO(peria): add tests if needed
#peria_add_test(field_test)
#peria_add_test(player_test)

