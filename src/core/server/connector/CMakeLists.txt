cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_server_connector
            connector.cc
            connector_manager_posix.cc
            human_connector.cc
            pipe_connector.cc)
