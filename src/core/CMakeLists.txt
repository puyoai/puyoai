cmake_minimum_required(VERSION 2.8)

add_subdirectory(algorithm)
add_subdirectory(client)
add_subdirectory(field)
add_subdirectory(server)

if(USE_HTTPD)
  add_subdirectory(httpd)
endif()

# ----------------------------------------------------------------------
# library

add_library(puyoai_core STATIC
            ctrl.cc frame_data.cc key.cc key_set.cc kumipuyo.cc
            plain_field.cc puyo_color.cc real_color.cc sequence_generator.cc)

# ----------------------------------------------------------------------
# tests

function(puyoai_core_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_core)
    puyoai_target_link_libraries(${target}_test)
    add_test(check-${target}_test ${target}_test)
endfunction()

puyoai_core_add_test(ctrl)
puyoai_core_add_test(key_set)
puyoai_core_add_test(kumipuyo)
puyoai_core_add_test(puyo_color)
puyoai_core_add_test(sequence_generator)
