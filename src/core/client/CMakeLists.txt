cmake_minimum_required(VERSION 2.8)

add_subdirectory(ai)

add_library(puyoai_core_client
            client_connector.cc)
