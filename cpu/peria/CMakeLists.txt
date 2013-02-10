cmake_minimum_required(VERSION 2.8)

cpu_setup("peria")

puyoai_add_cxx_flags("-Wall -Wextra")

add_library(peria_lib decision.cc field.cc game.cc player.cc)

function(peria_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_libraries(${exe} peria_lib)
  cpu_target_link_common_libraries(${exe})
endfunction()

function(peria_add_test exe)
  peria_add_executable(${exe} ${exe}.cc)
  cpu_target_link_libraries(${exe} ${GTEST_BOTH_LIBRARIES})
  if(NOT ARGV1)
    cpu_add_test(${exe})
  endif()
endfunction()

peria_add_executable(peria main.cc)

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})

  # TODO(peria): add tests if needed
  peria_add_test(field_test)
  peria_add_test(player_test)
endif()
