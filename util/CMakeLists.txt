cmake_minimum_required(VERSION 2.8)

include_directories(..)
add_library(puyoai_util STATIC tsc.cc)

option(TSC_ALT_IMPL "Flag to use alternative timestamp counter implementation" OFF)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/Config.h)
