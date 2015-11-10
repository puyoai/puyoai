cmake_minimum_required(VERSION 2.8)

# ----------------------------------------------------------------------
# google test
# ----------------------------------------------------------------------

add_subdirectory(googletest/googletest)

# ----------------------------------------------------------------------
# jsoncpp
# ----------------------------------------------------------------------

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gen)
add_custom_command(OUTPUT
  ${CMAKE_BINARY_DIR}/gen/jsoncpp/jsoncpp.cpp
  ${CMAKE_BINARY_DIR}/gen/jsoncpp/json/json.h
  ${CMAKE_BINARY_DIR}/gen/jsoncpp/json/json-forwards.h
  COMMAND
  python -u ${CMAKE_SOURCE_DIR}/third_party/jsoncpp/amalgamate.py
         -t ${CMAKE_SOURCE_DIR}/third_party/jsoncpp
         -s ${CMAKE_BINARY_DIR}/gen/jsoncpp/jsoncpp.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -Wno-error")
add_library(puyoai_third_party_jsoncpp
            ${CMAKE_BINARY_DIR}/gen/jsoncpp/jsoncpp.cpp)

# ----------------------------------------------------------------------
# tinytoml
# ----------------------------------------------------------------------
