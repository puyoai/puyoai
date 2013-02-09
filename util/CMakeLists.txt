cmake_minimum_required(VERSION 2.8)

include_directories(..)

function(puyoai_util_add_test target)
  add_executable(${target} ${target}.cc)
  target_link_libraries(${target} ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(${target} puyoai_core)
  target_link_libraries(${target} puyoai_util)
  puyoai_target_link_libraries(${target})
  add_test(check-${target} ${target})
endfunction()

add_library(puyoai_util STATIC tsc.cc)

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})

  puyoai_util_add_test(field_util_test)
endif()
