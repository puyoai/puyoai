cmake_minimum_required(VERSION 2.8)

cpu_setup("mayah")

add_library(mayah_lib
            ai_routine.cc feature_parameter.cc evaluator.cc gazer.cc)

function(mayah_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_libraries(${exe} mayah_lib)
  cpu_target_link_common_libraries(${exe})
endfunction()

function(mayah_add_test exe)
  mayah_add_executable(${exe} ${exe}.cc)
  cpu_target_link_libraries(${exe} gtest gtest_main)
  if(NOT ARGV1)
    cpu_add_test(${exe})
  endif()
endfunction()

mayah_add_executable(mayah_cpu main.cc)
mayah_add_executable(interactive interactive.cc)
#mayah_add_executable(learn learn.cc)
#mayah_add_executable(solver solver.cc)
#mayah_add_executable(solo solo.cc)

cpu_add_runner(run.sh)
cpu_add_runner(feature.txt)

mayah_add_test(gazer_test)

