cmake_minimum_required(VERSION 2.8)

function(puyoai_experimental_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_core)
    target_link_libraries(${target}_test puyoai_base)
    puyoai_target_link_libraries(${target}_test)
endfunction()

puyoai_experimental_add_test(drop)
puyoai_experimental_add_test(popcount)
