cmake_minimum_required(VERSION 2.8)

cpu_setup("yhayashi")

cpu_add_executable(main main.cc)
cpu_target_link_common_libraries(main)
