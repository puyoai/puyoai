cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_plan
            plan.cc)

# ----------------------------------------------------------------------
# test

function(puyoai_core_plan_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_core_plan)
    target_link_libraries(${target}_test puyoai_core)
    target_link_libraries(${target}_test puyoai_base)
    puyoai_target_link_libraries(${target}_test)
    if(NOT ARGV1)
        add_test(check-${target}_test ${target}_test)
    endif()
endfunction()

puyoai_core_plan_add_test(plan)

puyoai_core_plan_add_test(plan_performance 1)
