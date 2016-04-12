cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_client_connector
            client_connector.cc
            stdio_client_connector.cc
            socket_client_connector.cc)
