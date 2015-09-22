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

find_package(PkgConfig)

# TODO(mayah): gflags does not provide pkg-config anymore?
# pkg_search_module for libgflags doesn't work with Mac homebrew (2015-04-16).
find_library(LIB_GFLAGS gflags)
pkg_search_module(GLOG REQUIRED libglog)

pkg_search_module(SDL2 sdl2)
pkg_search_module(SDL2_TTF SDL2_ttf)
pkg_search_module(SDL2_IMAGE SDL2_image)
if(SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND)
    set(USE_SDL2 1)
endif()

find_library(LIB_MICROHTTPD microhttpd)
if(LIB_MICROHTTPD)
    set(USE_HTTPD 1)
endif()

if(PKG_CONFIG_FOUND)
    pkg_check_modules(FFMPEG libswscale libavformat libavcodec libavutil)

    # We don't support V4L2 yet.
    find_library(V4L2_LIBRARY v4l2)
endif()

# TODO(mayah): Why not use pkg_search_module?
find_library(LIB_USB NAMES usb-1.0)
if(LIB_USB AND USE_SDL2 AND FFMPEG_FOUND)
  set(BUILD_CAPTURE 1)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_library(COCOA_LIBRARY Cocoa)
endif()

# ----------------------------------------------------------------------
# Set include directories, c++ options, etc.

function(puyoai_add_cxx_flags flags)
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
    else()
        set(CMAKE_CXX_FLAGS "${flags} ${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
    endif()
endfunction()

if(CYGWIN AND (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU"))
    puyoai_add_cxx_flags("-Wall -Wextra -std=gnu++11")
else()
    puyoai_add_cxx_flags("-Wall -Wextra -std=c++11")
endif()
puyoai_add_cxx_flags("-Wno-error=deprecated-declarations")
puyoai_add_cxx_flags("-Wno-error=missing-field-initializers")
puyoai_add_cxx_flags("-Wno-error=sign-compare")
puyoai_add_cxx_flags("-march=native")

puyoai_add_cxx_flags("-DSRC_DIR=\\\"${CMAKE_SOURCE_DIR}\\\"")
puyoai_add_cxx_flags("-DTESTDATA_DIR=\\\"${CMAKE_SOURCE_DIR}/../test_resources\\\"")
puyoai_add_cxx_flags("-DDATA_DIR=\\\"${CMAKE_SOURCE_DIR}/../data\\\"")
puyoai_add_cxx_flags("-DRECOGNITION_DIR=\\\"${CMAKE_SOURCE_DIR}/../recognition\\\"")

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
if(${build_type} MATCHES release)
    puyoai_add_cxx_flags("-DNOINLINE_UNLESS_RELEASE=")
endif()

include_directories(.)

add_subdirectory(third_party/gtest-1.7.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_subdirectory(third_party/jsoncpp)
include_directories(third_party/jsoncpp)

include_directories(third_party/tinytoml/include)
include_directories(${GLOG_INCLUDE_DIRS})

if(USE_SDL2)
    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${SDL2_TTF_INCLUDE_DIRS})
    include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
    include_directories(${FFMPEG_INCLUDE_DIR})
    puyoai_add_cxx_flags("-DUSE_SDL2")
endif()
if(USE_HTTPD)
    puyoai_add_cxx_flags("-DUSE_HTTPD")
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/internal AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/internal)
    set(USE_INTERNAL 1)
    puyoai_add_cxx_flags("-DUSE_INTERNAL=1")
endif()
if(USE_INTERNAL AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(USE_AUDIO_COMMENTATOR 1)
    puyoai_add_cxx_flags("-DUSE_AUDIO_COMMENTATOR=1")
endif()

# ----------------------------------------------------------------------
# Add subdirectories

function(puyoai_target_link_libraries target)
    target_link_libraries(${target} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${target} ${LIB_GFLAGS})
    target_link_libraries(${target} ${GLOG_LIBRARIES})
endfunction()

add_subdirectory(base)
add_subdirectory(core)
add_subdirectory(cpu)
add_subdirectory(duel)
add_subdirectory(experimental)
add_subdirectory(recognition)
add_subdirectory(solver)

if(EXISTS ${CMAKE_SOURCE_DIR}/internal AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/internal)
    add_subdirectory(internal)
endif()

if(USE_SDL2)
    add_subdirectory(audio)
    add_subdirectory(gui)
    add_subdirectory(tool)
endif()

if(BUILD_CAPTURE)
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

if(USE_SDL2)
    puyoai_message("SDL2, SDL2_image and SDL2_ttf found - GUI will be ENABLED")
else()
    puyoai_message("SDL2, SDL2_image and/or SDL2_ttf not found - GUI will be DISABLED")
endif()

if(USE_HTTPD)
    puyoai_message("HTTPD is enabled")
endif()

if(BUILD_CAPTURE)
    puyoai_message("Will build capture/")
else()
    puyoai_message("Won't build capture/ - see capture/README")
endif()

message(STATUS "*********************************************************")
message(STATUS "")
