cmake_minimum_required(VERSION 2.8)

cpu_setup("mayah")

add_library(mayah_lib
            decision_book.cc
            evaluator.cc
            evaluation_feature.cc
            evaluation_parameter.cc
            feature_collector.cc
            gazer.cc
            mayah_ai.cc
            pattern_book.cc
            pattern_rensa_detector.cc)

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
mayah_add_executable(solver solver_main.cc)
mayah_add_executable(tweaker tweaker.cc)

cpu_add_runner(run.sh)
cpu_add_runner(run_v.sh)
cpu_add_runner(run_without_joseki.sh)

mayah_add_test(decision_book_test)
mayah_add_test(evaluator_test)
mayah_add_test(evaluation_parameter_test)
mayah_add_test(gazer_test)
mayah_add_test(mayah_ai_test)
mayah_add_test(mayah_ai_situation_test)
mayah_add_test(pattern_rensa_detector_test)

mayah_add_test(mayah_ai_performance_test 1)
