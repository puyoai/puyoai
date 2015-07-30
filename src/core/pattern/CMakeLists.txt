cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_pattern
            bijection_matcher.cc
            field_pattern.cc
            pattern_matcher.cc)

# ----------------------------------------------------------------------
# test

function(puyoai_core_pattern_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_core_pattern)
    target_link_libraries(${target}_test puyoai_core)
    target_link_libraries(${target}_test puyoai_base)
    puyoai_target_link_libraries(${target}_test)
    if(NOT ARGV1)
        add_test(check-${target}_test ${target}_test)
    endif()
endfunction()

puyoai_core_pattern_add_test(field_pattern)
puyoai_core_pattern_add_test(pattern_matcher)
