cmake_minimum_required(VERSION 2.8)

add_subdirectory(socket)

if(USE_HTTPD)
    add_subdirectory(httpd)
endif()
