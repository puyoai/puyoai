project(puyoai)

# ----------------------------------------------------------------------
# cmake settings

cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

enable_testing()

# ----------------------------------------------------------------------
# Check packages, libraries, etc.

find_package(Threads)

find_path(GFLAGS_PATH gflags/gflags.h)
if(GFLAGS_PATH)
  find_library(GFLAGS_LIBRARY gflags)
  if(NOT GFLAGS_LIBRARY)
    message(STATUS "Could NOT find gflags (GFLAGS_LIBRARY)")
  endif()
else()
  message(STATUS "Could NOT find gflags (GFLAGS_PATH)")
endif()

find_path(GLOG_PATH glog/logging.h)
if(GLOG_PATH)
  find_library(GLOG_LIBRARY glog)
  if(NOT GLOG_LIBRARY)
    message(STATUS "Could NOT find glog (GLOG_LIBRARY)")
  endif()
else()
  message(STATUS "Could NOT find glog (GLOG_PATH)")
endif()

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

find_package(PkgConfig)

pkg_search_module(SDL2 sdl2)
pkg_search_module(SDL2_TTF SDL2_ttf)
pkg_search_module(SDL2_IMAGE SDL2_image)
if(SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND)
  set(USE_SDL2 1)
endif()

# TODO(mayah): Check the existence of libmicrohttpd.
find_library(LIB_MICROHTTPD microhttpd)
if(LIB_MICROHTTPD)
  set(USE_HTTPD 1)
endif()

if(PKG_CONFIG_FOUND)
  pkg_check_modules(FFMPEG libswscale libavformat libavcodec libavutil)
  find_library(V4L2_LIBRARY v4l2)
endif()

# TODO(mayah): Why not use pkg_search_module?
find_library(LIB_USB NAMES usb-1.0)
find_library(LIB_GCRYPT NAMES gcrypt)

# ----------------------------------------------------------------------
# Set include directories, c++ options, etc.

function(puyoai_add_cxx_flags flags)
  if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}" PARENT_SCOPE)
  endif()
endfunction()

puyoai_add_cxx_flags("-Wall -Wextra -std=c++11")
puyoai_add_cxx_flags("-DTESTDATA_DIR=\\\"${CMAKE_SOURCE_DIR}/../testdata\\\"")
puyoai_add_cxx_flags("-DDATA_DIR=\\\"${CMAKE_SOURCE_DIR}/../data\\\"")

include_directories(.)

add_subdirectory(third_party/gtest-1.7.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

if(USE_SDL2)
  include_directories(${SDL2_INCLUDE_DIRS})
  include_directories(${SDL2_TTF_INCLUDE_DIRS})
  include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
  include_directories(${FFMPEG_INCLUDE_DIR})
  puyoai_add_cxx_flags("-DUSE_SDL2")
endif()

# ----------------------------------------------------------------------
# Add subdirectories

function(puyoai_target_link_libraries target)
  target_link_libraries(${target} ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(${target} ${GFLAGS_LIBRARY})
  target_link_libraries(${target} ${GLOG_LIBRARY})
endfunction()

add_subdirectory(base)
add_subdirectory(core)
add_subdirectory(cpu)
add_subdirectory(duel)

if(USE_SDL2)
  add_subdirectory(gui)
endif()

if(USE_GFLAGS_AND_GLOG AND USE_SDL2 AND FFMPEG_FOUND)
  set(BUILD_CAPTURE 1)
  add_subdirectory(capture)
  add_subdirectory(wii)
endif()

# ----------------------------------------------------------------------
# For testing

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

if(USE_SDL2)
  puyoai_message("SDL2, SDL2_image and SDL2_ttf found - GUI will be ENABLED")
else()
  puyoai_message("SDL2, SDL2_image and/or SDL2_ttf not found - GUI will be DISABLED")
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

