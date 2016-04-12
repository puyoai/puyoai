cmake_minimum_required(VERSION 2.8)

if(MSVC)
    set(pipe_connector_os_cc pipe_connector_win.cc)
else()
    set(pipe_connector_os_cc pipe_connector_posix.cc)
endif()

add_library(puyoai_core_server_connector
            connector_manager.cc
            human_connector.cc
            pipe_connector.cc
            server_connector.cc
            socket_connector.cc
            ${pipe_connector_os_cc})
