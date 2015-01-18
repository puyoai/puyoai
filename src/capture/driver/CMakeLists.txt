cmake_minimum_required(VERSION 2.8)

if(LIB_USB)
    add_library(puyoai_capture_driver_syntek syntek.cc)
endif()
