cmake_minimum_required(VERSION 2.8)

add_library(puyoai_core_server_connector
            connector.cc connector_manager_linux.cc received_data.cc)
