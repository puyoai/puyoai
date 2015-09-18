cmake_minimum_required(VERSION 2.8)

add_library(puyoai_base
            executor.cc
            file.cc
            time.cc
            time_stamp_counter.cc
            strings.cc
            wait_group.cc)

# ----------------------------------------------------------------------

function(puyoai_base_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_base)
    puyoai_target_link_libraries(${target}_test)
endfunction()

puyoai_base_add_test(bmi)
puyoai_base_add_test(file)
puyoai_base_add_test(sse)
puyoai_base_add_test(strings)
puyoai_base_add_test(small_int_set)
