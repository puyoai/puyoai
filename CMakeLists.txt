project(puyoai)

cmake_minimum_required(VERSION 2.8)

enable_testing()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_package(Threads)

find_path(GFLAGS_PATH gflags/gflags.h)
find_library(GFLAGS_LIBRARY gflags)
find_path(GLOG_PATH glog/logging.h)
find_library(GLOG_LIBRARY glog)

if(GFLAGS_PATH AND GFLAGS_LIBRARY AND GLOG_PATH AND GLOG_LIBRARY)
  set(USE_GFLAGS_AND_GLOG 1)
  include_directories(${GFLAGS_PATH})
  include_directories(${GLOG_PATH})
else()
  include_directories(third_party)
  add_subdirectory(third_party)
  set(GFLAGS_LIBRARY gflags)
  set(GLOG_LIBRARY glog)
endif()

function(puyoai_target_link_libraries target)
  target_link_libraries(${target} ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(${target} ${GFLAGS_LIBRARY})
  target_link_libraries(${target} ${GLOG_LIBRARY})
endfunction()

function(puyoai_add_cxx_flags flags)
  if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}" PARENT_SCOPE)
  endif()
endfunction()

find_package(GTest)

find_package(SDL)
find_package(SDL_image)
find_package(SDL_ttf)
if(SDL_FOUND AND SDLTTF_FOUND)
  set(USE_SDL 1)
endif()

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(FFMPEG libswscale libavformat libavcodec libavutil)
  find_library(V4L2_LIBRARY v4l2)
endif()

add_subdirectory(core)
add_subdirectory(cpu)
add_subdirectory(duel)
add_subdirectory(util)

if(USE_GFLAGS_AND_GLOG AND USE_SDL AND FFMPEG_FOUND)
  set(BUILD_CAPTURE 1)
  add_subdirectory(capture)
endif()

function(puyoai_message msg)
  message(STATUS "puyoai: ${msg}")
endfunction()

message(STATUS "")
message(STATUS "*************** puyoai configuration note ***************")

if(USE_GFLAGS_AND_GLOG)
  puyoai_message("Use gflags and glog installed")
else()
  puyoai_message("Use dummy gflags and glog")
endif()

if(USE_SDL)
  puyoai_message("SDL and SDL_ttf found - GUI will be ENABLED")
else()
  puyoai_message("SDL and SDL_ttf not found - GUI will be DISABLED")
endif()

if(GTEST_FOUND)
  puyoai_message("gtest found - Tests will be ENABLED")
else()
  puyoai_message("gtest not found - Tests will be DISABLED")
endif()

if(BUILD_CAPTURE)
  puyoai_message("Will build capture/")
  if(NOT V4L2_LIBRARY)
    puyoai_message("but v4l2 is missing - see capture/README")
  endif()
else()
  puyoai_message("Won't build capture/ - see capture/README")
endif()

message(STATUS "*********************************************************")
message(STATUS "")
