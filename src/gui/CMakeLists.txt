cmake_minimum_required(VERSION 2.8)

add_library(puyoai_gui
            bounding_box.cc bounding_box_drawer.cc
            color_map.cc commentator.cc
            field_drawer.cc fps_drawer.cc main_window.cc screen.cc
            util.cc SDL_kanji.c SDL_prims.c)

function(puyoai_gui_add_test target)
  add_executable(${target}_test ${target}_test.cc)
  target_link_libraries(${target}_test gtest gtest_main)
  target_link_libraries(${target}_test puyoai_base)
  target_link_libraries(${target}_test puyoai_core)
  target_link_libraries(${target}_test puyoai_duel)
  target_link_libraries(${target}_test puyoai_gui)
  target_link_libraries(${target}_test ${SDL2_LIBRARIES})
  target_link_libraries(${target}_test ${SDL2_TTF_LIBRARIES})
  puyoai_target_link_libraries(${target}_test)
  add_test(check-${target}_test ${target}_test)
endfunction()

puyoai_gui_add_test(commentator)

