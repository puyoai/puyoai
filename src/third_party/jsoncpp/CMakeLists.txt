cmake_minimum_required(VERSION 2.8)

set(LIB_JSONCPP puyoai_lib_jsoncpp PARENT_SCOPE)

include_directories(.)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")
add_library(puyoai_lib_jsoncpp
            jsoncpp.cpp)
