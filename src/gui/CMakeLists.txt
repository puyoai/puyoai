cmake_minimum_required(VERSION 2.8)

add_library(puyoai_gui
            SDL_kanji.c
            SDL_prims.c
            bounding_box.cc
            bounding_box_drawer.cc
            commentator_drawer.cc
            decision_drawer.cc
            field_drawer.cc
            fps_drawer.cc
            frame_number_drawer.cc
            human_connector_key_listener.cc
            main_window.cc
            screen.cc
            pixel_color.cc
            player_ui.cc
            user_event_drawer.cc
            util.cc)

function(puyoai_gui_add_test target)
    add_executable(${target}_test ${target}_test.cc)
    target_link_libraries(${target}_test gtest gtest_main)
    target_link_libraries(${target}_test puyoai_base)
    target_link_libraries(${target}_test puyoai_core)
    target_link_libraries(${target}_test puyoai_gui)
    target_link_libraries(${target}_test ${SDL2_LIBRARIES})
    target_link_libraries(${target}_test ${SDL2_TTF_LIBRARIES})
    target_link_libraries(${target}_test ${SDL2_IMAGE_LIBRARIES})
    puyoai_target_link_libraries(${target}_test)
    add_test(check-${target}_test ${target}_test)
endfunction()

puyoai_gui_add_test(box)
