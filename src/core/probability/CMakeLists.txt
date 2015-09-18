cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_probability
            column_puyo_list_probability.cc
            puyo_set_probability.cc
            puyo_set.cc)

# ----------------------------------------------------------------------
# test

function(puyoai_core_probability_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_core_probability)
    target_link_libraries(${target}_test puyoai_core)
    target_link_libraries(${target}_test puyoai_base)
    puyoai_target_link_libraries(${target}_test)
    if(NOT ARGV1)
        add_test(check-${target}_test ${target}_test)
    endif()
endfunction()

puyoai_core_probability_add_test(column_puyo_list_probability)
puyoai_core_probability_add_test(puyo_set_probability)
puyoai_core_probability_add_test(puyo_set)
