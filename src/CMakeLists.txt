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

if(MSVC)
    # TODO(peria): This configuration is not exptected to work for everyone.
    set(glog_DIR "C:/Program Files/google-glog/lib/cmake/glog")
    find_package(gflags REQUIRED)
    find_package(glog REQUIRED)
    set(LIB_GFLAGS ${gflags_LIBRARIES})
    set(GLOG_LIBRARIES ${glog_LIBRARIES})
else()
    # TODO(mayah): gflags does not provide pkg-config anymore?
    # pkg_search_module for libgflags doesn't work with Mac homebrew (2015-04-16).
    find_library(LIB_GFLAGS gflags)
    pkg_search_module(GLOG REQUIRED libglog)
endif()

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

# TODO(peria): Enable to use TCP sockets with MSVC
if(NOT MSVC)
    set(USE_TCP 1)
endif()

# ----------------------------------------------------------------------
# Set include directories, c++ options, etc.

if(${CMAKE_VERSION} VERSION_LESS 2.8.12)
    # This is a workaround for TravisCI.
    function(add_compile_options option)
        add_definitions(${option} ${ARGN})
    endfunction()
endif()

if(MSVC)
    add_compile_options("/MD$<$<CONFIG:Debug>:d>")
    add_compile_options("/MP")  # Compile in parallel
    # Suppress some warnigs about implicit conversions.
    add_compile_options("/wd4244")  # from uint64 to int
    add_compile_options("/wd4267")  # from size_t to int
    add_compile_options("/wd4800")  # from int to bool

    # Always include build_config.h
    add_compile_options("/FI ${CMAKE_SOURCE_DIR}/build/build_config.h")
else()
    if(CYGWIN AND (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU"))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11" CACHE STRING "" FORCE)
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" CACHE STRING "" FORCE)
    endif()
    add_compile_options("-march=native")

    add_compile_options("-Wall")
    add_compile_options("-Wextra")
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_compile_options("-Werror")
    endif()

    # Always include build/build_config.h
    add_compile_options("-include" "${CMAKE_SOURCE_DIR}/build/build_config.h")
endif()

add_definitions(-DSRC_DIR="${CMAKE_SOURCE_DIR}")
add_definitions(-DTESTDATA_DIR="${CMAKE_SOURCE_DIR}/../test_resources")
add_definitions(-DDATA_DIR="${CMAKE_SOURCE_DIR}/../data")

if(MSVC)
  add_definitions(-DGOOGLE_GLOG_DLL_DECL=)
  add_definitions(-DNOMINMAX)
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
if(${build_type} MATCHES release)
    add_definitions(-DNOINLINE_UNLESS_RELEASE=)
endif()

add_subdirectory(third_party)

include_directories(.)
include_directories(${CMAKE_BINARY_DIR}/gen/jsoncpp)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(third_party/tinytoml/include)
include_directories(${gflags_INCLUDE_DIR})
include_directories(${glog_INCLUDE_DIRS})

if(USE_SDL2)
    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${SDL2_TTF_INCLUDE_DIRS})
    include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
    include_directories(${FFMPEG_INCLUDE_DIR})
    add_definitions(-DUSE_SDL2)
endif()

if(USE_HTTPD)
    add_definitions(-DUSE_HTTPD)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/internal AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/internal)
    set(USE_INTERNAL 1)
    add_definitions(-DUSE_INTERNAL=1)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(USE_AUDIO_COMMENTATOR 1)
        add_definitions(-DUSE_AUDIO_COMMENTATOR=1)
    endif()
endif()

if(USE_TCP)
    add_definitions(-DUSE_TCP=1)
endif()

# ----------------------------------------------------------------------
# Add subdirectories

function(puyoai_target_link_libraries target)
    target_link_libraries(${target} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${target} ${LIB_GFLAGS})
    target_link_libraries(${target} ${GLOG_LIBRARIES})
endfunction()

add_subdirectory(base)
add_subdirectory(build)
add_subdirectory(core)
add_subdirectory(cpu)
add_subdirectory(duel)
add_subdirectory(experimental)
add_subdirectory(learning)
add_subdirectory(net)
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
# Display configurations

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
else()
    puyoai_message("HTTPD is NOT enabled")
endif()

if(BUILD_CAPTURE)
    puyoai_message("Will build capture/")
else()
    puyoai_message("Won't build capture/ - see capture/README")
endif()

message(STATUS "*********************************************************")
message(STATUS "")
