cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_rensa_tracker
            rensa_chain_tracker.cc
            rensa_coef_tracker.cc
            rensa_vanishing_position_tracker.cc)

# ----------------------------------------------------------------------
# test

function(puyoai_core_rensa_tracker_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_core_rensa_tracker)
    target_link_libraries(${target}_test puyoai_core)
    target_link_libraries(${target}_test puyoai_base)
    puyoai_target_link_libraries(${target}_test)
    if(NOT ARGV1)
        add_test(check-${target}_test ${target}_test)
    endif()
endfunction()

puyoai_core_rensa_tracker_add_test(rensa_chain_tracker)
puyoai_core_rensa_tracker_add_test(rensa_coef_tracker)
puyoai_core_rensa_tracker_add_test(rensa_composite_tracker)
puyoai_core_rensa_tracker_add_test(rensa_existing_position_tracker)
puyoai_core_rensa_tracker_add_test(rensa_vanishing_position_tracker)
puyoai_core_rensa_tracker_add_test(rensa_last_vanished_position_tracker)

puyoai_core_rensa_tracker_add_test(rensa_tracker_performance 1)
