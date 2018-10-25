cmake_minimum_required(VERSION 2.8)

if(MSVC)
    set(unix_domain_cc "")
else()
    set(unix_domain_cc 
        unix_domain_client_socket.cc
        unix_domain_server_socket.cc
        unix_domain_socket.cc)
endif()

add_library(puyoai_net_socket
            socket.cc
            socket_factory.cc
            tcp_client_socket.cc
            tcp_server_socket.cc
            tcp_socket.cc
            ${unix_domain_cc})

if(MSVC)
    target_link_libraries(puyoai_net_socket
                          ws2_32.lib)
endif()
