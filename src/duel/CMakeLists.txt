cmake_minimum_required(VERSION 2.8)

if(USE_HTTPD)
  set(HTTPD_SOURCE http_server.cc)
endif()

add_library(puyoai_duel
            cui.cc duel_server.cc field_realtime.cc
            frame_context.cc game_state.cc
            puyofu_recorder.cc
            ${HTTPD_SOURCE})

add_executable(duel main.cc)

if(USE_SDL2)
  target_link_libraries(duel puyoai_gui)
endif()
target_link_libraries(duel puyoai_duel)
target_link_libraries(duel puyoai_core_algorithm)
target_link_libraries(duel puyoai_core_server_connector)
target_link_libraries(duel puyoai_core_field)
target_link_libraries(duel puyoai_core)
target_link_libraries(duel puyoai_base)
if(USE_SDL2)
  target_link_libraries(duel puyoai_gui)
  target_link_libraries(duel ${SDL2_LIBRARIES})
  target_link_libraries(duel ${SDL2_IMAGE_LIBRARIES})
  target_link_libraries(duel ${SDL2_TTF_LIBRARIES})
endif()
if(USE_HTTPD)
  target_link_libraries(duel ${LIB_MICROHTTPD})
endif()
puyoai_target_link_libraries(duel)

# ----------------------------------------------------------------------

function(puyoai_duel_add_test target)
  add_executable(${target}_test ${target}_test.cc)
  target_link_libraries(${target}_test gtest gtest_main)
  target_link_libraries(${target}_test puyoai_duel)
  target_link_libraries(${target}_test puyoai_core_field)
  target_link_libraries(${target}_test puyoai_core)
  puyoai_target_link_libraries(${target}_test)
  add_test(check-${target}_test ${target}_test)
endfunction()

puyoai_duel_add_test(field_realtime)

