cmake_minimum_required(VERSION 2.8)

cpu_setup("sample")

cpu_add_executable(sample sample.cc)
cpu_target_link_common_libraries(sample)
