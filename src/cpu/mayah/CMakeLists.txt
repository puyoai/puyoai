cmake_minimum_required(VERSION 2.8)

cpu_setup("mayah")

add_library(mayah_lib
            book_field.cc book_reader.cc
            evaluator.cc feature_parameter.cc gazer.cc mayah_ai.cc util.cc)

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
mayah_add_executable(learn learn.cc)
mayah_add_executable(tweaker tweaker.cc)

cpu_add_runner(run.sh)

cpu_add_runner(feature.txt)
cpu_add_runner(run_for_tweaker_1p.sh)
cpu_add_runner(run_for_tweaker_2p.sh)
cpu_add_runner(auto_tweaker.go)

mayah_add_test(evaluator_test)
mayah_add_test(book_field_test)
mayah_add_test(gazer_test)
mayah_add_test(mayah_ai_test)

mayah_add_test(mayah_ai_performance_test 1)
