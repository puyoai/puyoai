cmake_minimum_required(VERSION 2.8)

cpu_setup("hidehiko")

cpu_add_executable(cpu main.cc)
cpu_add_runner(run.sh)
cpu_target_link_common_libraries(cpu)
