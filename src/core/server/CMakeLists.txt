cmake_minimum_required(VERSION 2.8)

add_subdirectory(connector)

add_library(puyoai_core_server
            commentator.cc
            game_state.cc
            game_state_recorder.cc)

function(puyoai_core_server_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_base)
    target_link_libraries(${target}_test puyoai_core)
    puyoai_target_link_libraries(${target}_test)
    add_test(check-${target}_test ${target}_test)
endfunction()

puyoai_core_server_add_test(commentator)
