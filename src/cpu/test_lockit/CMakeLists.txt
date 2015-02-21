cmake_minimum_required(VERSION 2.8)

cpu_setup("test_lockit")

function(test_lockit_add_executable exe)
    cpu_add_executable(${exe} ${ARGN})
    cpu_target_link_common_libraries(${exe})
endfunction()

test_lockit_add_executable(nidub read.cc coma.cc cpu.cc nidub.cpp)
test_lockit_add_executable(rendaS9 read.cc coma.cc cpu.cc rendaS9.cpp)
test_lockit_add_executable(rendaGS9 read.cc coma.cc cpu.cc rendaGS9.cpp)

cpu_add_runner(nidub.sh)
cpu_add_runner(rendaS9.sh)
cpu_add_runner(rendaGS9.sh)
