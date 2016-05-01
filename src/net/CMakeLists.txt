cmake_minimum_required(VERSION 2.8)

if(USE_TCP)
    add_subdirectory(socket)
endif()

if(USE_HTTPD)
    add_subdirectory(httpd)
endif()
