cmake_minimum_required(VERSION 2.8)

include_directories(..)

add_library(puyoai_core STATIC
            ctrl.cc field.cc)

function(puyoai_core_add_test target)
  add_executable(${target}_test ${target}_test.cc)
  target_link_libraries(${target}_test ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(${target}_test puyoai_core)
  puyoai_target_link_libraries(${target}_test)
  add_test(check-${target}_test ${target}_test)
endfunction()

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})

  puyoai_core_add_test(ctrl)
  puyoai_core_add_test(field)
  # TODO(hamaji): Currently, this test is broken.
  # puyoai_core_add_test(performance)
endif()
