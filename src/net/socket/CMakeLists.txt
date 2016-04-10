cmake_minimum_required(VERSION 2.8)

add_library(puyoai_net_socket STATIC
            socket.cc
            socket_factory.cc
            tcp_client_socket.cc
            tcp_server_socket.cc
            tcp_socket.cc
            unix_domain_client_socket.cc
            unix_domain_server_socket.cc
            unix_domain_socket.cc)
