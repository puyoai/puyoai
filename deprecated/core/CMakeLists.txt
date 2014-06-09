cmake_minimum_required(VERSION 2.8)

include_directories(..)

add_library(puyoai_core STATIC field.cc puyo.cc)

function(puyoai_core_add_test target)
  add_executable(${target}_test ${target}_test.cc)
  target_link_libraries(${target}_test gtest gtest_main)
  target_link_libraries(${target}_test puyoai_core)
  puyoai_target_link_libraries(${target}_test)
  add_test(check-${target}_test ${target}_test)
endfunction()

# --- Adds test targets.
include_directories(${gtest_SOURCE_DIR}/include
                    ${gtest_SOURCE_DIR})
puyoai_core_add_test(ctrl)
puyoai_core_add_test(field)
# TODO(hamaji): Currently, this test is broken.
# puyoai_core_add_test(performance)

