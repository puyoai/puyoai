cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_connector
            socket_connector_impl.cc
            stdio_connector_impl.cc)
