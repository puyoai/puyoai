cmake_minimum_required(VERSION 2.8)

add_library(puyoai_capture
            analyzer.cc analyzer_result_drawer.cc
            capture.cc color.cc images_source.cc movie_source.cc screen_shot_saver.cc source.cc
            movie_source_key_listener.cc
            real_color_field.cc)

if(V4L2_LIBRARY)
  puyoai_add_cxx_flags("-DUSE_V4L2")
  add_library(puyoai_capture_viddev viddev_source.cc)
endif()

if(LIB_USB AND LIB_GCRYPT)
  puyoai_add_cxx_flags("-DUSE_SOMAGIC")
  add_library(puyoai_capture_somagic somagic_analyzer.cc somagic_source.cc)
endif()

# ----------------------------------------------------------------------

function(capture_add_executable exe)
  add_executable(${exe} ${exe}.cc)
  if(LIB_USB AND LIB_GCRYPT)
    target_link_libraries(${exe} puyoai_capture_somagic)
  endif()
  target_link_libraries(${exe} puyoai_capture)
  target_link_libraries(${exe} puyoai_gui)
  target_link_libraries(${exe} puyoai_core)
  target_link_libraries(${exe} puyoai_duel)
  target_link_libraries(${exe} puyoai_base)
  if(V4L2_LIBRARY)
    target_link_libraries(${exe} puyoai_capture_viddev)
  endif()
  puyoai_target_link_libraries(${exe})
  target_link_libraries(${exe} ${SDL2_LIBRARIES})
  target_link_libraries(${exe} ${SDL2_TTF_LIBRARIES})
  target_link_libraries(${exe} ${SDL2_IMAGE_LIBRARIES})
  target_link_libraries(${exe} ${FFMPEG_LIBRARIES})
  if(V4L2_LIBRARY)
    target_link_libraries(${exe} ${V4L2_LIBRARY})
  endif()
  if(LIB_USB AND LIB_GCRYPT)
    target_link_libraries(${exe} ${LIB_USB} ${LIB_GCRYPT})
  endif()
endfunction()

capture_add_executable(parse_bmp)
capture_add_executable(parse_box)
# Disabled.
#capture_add_executable(parse_imgs)
capture_add_executable(parse_movie)
if(V4L2_LIBRARY)
# Disabled.
  capture_add_executable(parse_viddev)
endif()
if(LIB_USB AND LIB_GCRYPT)
  capture_add_executable(parse_somagic)
endif()

# ----------------------------------------------------------------------

function(capture_add_test exe)
  capture_add_executable(${exe})
  target_link_libraries(${exe} gtest gtest_main)
  add_test(check-${exe} ${exe})
endfunction()

capture_add_test(color_test)
capture_add_test(real_color_field_test)
capture_add_test(somagic_analyzer_test)
