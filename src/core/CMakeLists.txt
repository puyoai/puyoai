cmake_minimum_required(VERSION 2.8)

add_subdirectory(client)
add_subdirectory(pattern)
add_subdirectory(plan)
add_subdirectory(probability)
add_subdirectory(rensa)
add_subdirectory(rensa_tracker)
add_subdirectory(server)

if(USE_HTTPD)
    add_subdirectory(httpd)
endif()

# ----------------------------------------------------------------------
# library

add_library(puyoai_core STATIC
            bit_field.cc
            column_puyo_list.cc
            core_field.cc
            decision.cc
            field_bits.cc
            field_bits_256.cc
            field_pretty_printer.cc
            flags.cc
            frame_request.cc
            frame_response.cc
            game_result.cc
            key.cc
            key_set.cc
            key_set_seq.cc
            kumipuyo.cc
            kumipuyo_moving_state.cc
            kumipuyo_pos.cc
            kumipuyo_seq.cc
            kumipuyo_seq_generator.cc
            plain_field.cc
            puyo_color.cc
            puyo_controller.cc
            real_color.cc
            user_event.cc)

# ----------------------------------------------------------------------
# tests

function(puyoai_core_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_core)
    target_link_libraries(${target}_test puyoai_base)
    puyoai_target_link_libraries(${target}_test)
    if(NOT ARGV1)
        add_test(check-${target}_test ${target}_test)
    endif()
endfunction()

puyoai_core_add_test(bit_field)
puyoai_core_add_test(column_puyo_list)
puyoai_core_add_test(core_field)
puyoai_core_add_test(decision)
puyoai_core_add_test(field_bits)
puyoai_core_add_test(field_bits_256)
puyoai_core_add_test(field_checker)
puyoai_core_add_test(frame_response)
puyoai_core_add_test(frame_request)
puyoai_core_add_test(key_set)
puyoai_core_add_test(key_set_seq)
puyoai_core_add_test(kumipuyo)
puyoai_core_add_test(kumipuyo_moving_state)
puyoai_core_add_test(kumipuyo_pos)
puyoai_core_add_test(kumipuyo_seq)
puyoai_core_add_test(kumipuyo_seq_generator)
puyoai_core_add_test(plain_field)
puyoai_core_add_test(player_state)
puyoai_core_add_test(puyo_color)
puyoai_core_add_test(puyo_controller)
puyoai_core_add_test(rensa_result)

puyoai_core_add_test(bit_field_performance 1)
puyoai_core_add_test(field_performance 1)
puyoai_core_add_test(puyo_controller_performance 1)
