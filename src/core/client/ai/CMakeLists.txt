cmake_minimum_required(VERSION 2.8)

add_subdirectory(endless)
add_subdirectory(solver)

add_library(puyoai_core_client_ai
            ai.cc)

function(puyoai_client_ai_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_base)
    target_link_libraries(${target}_test puyoai_core_client_ai)
    target_link_libraries(${target}_test puyoai_core_client_connector)
    target_link_libraries(${target}_test puyoai_core)
    puyoai_target_link_libraries(${target}_test)
    add_test(check-${target}_test ${target}_test)
endfunction()

puyoai_client_ai_add_test(ai)
