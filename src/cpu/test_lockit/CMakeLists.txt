cmake_minimum_required(VERSION 2.8)

cpu_setup("test_lockit")

add_library(test_lockit_lib
            coma.cc
            cpu.cc
            read.cc)

function(test_lockit_add_executable exe)
    cpu_add_executable(${exe} ${ARGN})
    cpu_target_link_libraries(${exe} test_lockit_lib)
    cpu_target_link_common_libraries(${exe})
endfunction()

test_lockit_add_executable(nidub nidub.cpp)
test_lockit_add_executable(rendaS9 rendaS9.cpp)
test_lockit_add_executable(rendaGS9 rendaGS9.cpp)
test_lockit_add_executable(rendaGS9a rendaGS9a.cpp)

cpu_add_runner(nidub.sh)
cpu_add_runner(rendaS9.sh)
cpu_add_runner(rendaGS9.sh)
cpu_add_runner(rendaGS9.sh)
