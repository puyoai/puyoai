cmake_minimum_required(VERSION 2.8)

include_directories(..)

if (USE_SDL)
  set(GUI_SRCS SDL_kanji.c screen.cc commentator.cc)
  include_directories(${SDL_INCLUDE_DIR})
  include_directories(${SDLTTF_INCLUDE_DIR})
  set(CMAKE_CXX_FLAGS "-DUSE_SDL")
endif()

add_library(puyoai_duel
            connector.cc connector_manager_linux.cc field_realtime.cc
            game.cc game_log.cc gui.cc ojama_controller.cc ${GUI_SRCS})

add_executable(duel duel.cc)
if(USE_SDL)
  target_link_libraries(duel ${SDL_LIBRARY} ${SDLTTF_LIBRARY})
endif()
puyoai_target_link_libraries(duel)
target_link_libraries(duel puyoai_duel)
target_link_libraries(duel puyoai_core)

function(puyoai_duel_add_test target)
  add_executable(${target}_test ${target}_test.cc)
  target_link_libraries(${target}_test ${GTEST_BOTH_LIBRARIES})
  puyoai_target_link_libraries(${target}_test)
  target_link_libraries(${target}_test puyoai_duel)
  target_link_libraries(${target}_test puyoai_core)
  add_test(check-${target}_test ${target}_test)
endfunction()

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})

  puyoai_duel_add_test(field_realtime)

  if (USE_SDL)
    puyoai_duel_add_test(commentator)
    target_link_libraries(commentator_test ${SDL_LIBRARY} ${SDLTTF_LIBRARY})
  endif()
endif()
