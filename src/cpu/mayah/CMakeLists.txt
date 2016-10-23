cmake_minimum_required(VERSION 2.8)

cpu_setup("mayah")

add_library(mayah_evaluator_lib
            collected_score.cc
            evaluator.cc
            evaluation_feature.cc
            evaluation_mode.cc
            evaluation_parameter.cc
            move_evaluator.cc
            pattern_rensa_detector.cc
            rensa_evaluator.cc
            rensa_hand_tree.cc
            shape_evaluator.cc)

add_library(mayah_thinker_lib
            beam_thinker.cc
            pattern_thinker.cc
            rush_thinker.cc
            side_thinker.cc
            gazer.cc)

add_library(mayah_lib
            mayah_ai.cc
            mayah_base_ai.cc
            mixed_ai.cc
            yukina_ai.cc)

function(mayah_add_executable exe)
    cpu_add_executable(${exe} ${ARGN})
    cpu_target_link_libraries(${exe} mayah_lib)
    cpu_target_link_libraries(${exe} mayah_thinker_lib)
    cpu_target_link_libraries(${exe} mayah_evaluator_lib)
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
mayah_add_executable(yukina_cpu yukina.cc)

mayah_add_executable(interactive interactive.cc)
mayah_add_executable(solver solver_main.cc)
mayah_add_executable(tweaker tweaker.cc)

mayah_add_executable(experimental experimental.cc)

mayah_add_executable(gazer_example gazer_example.cc)

if(USE_HTTPD)
    mayah_add_executable(pattern_maker pattern_maker.cc)
endif()

cpu_add_runner(run.sh)
cpu_add_runner(run_v.sh)

cpu_add_runner(yukina.sh)
cpu_add_runner(yukina_v.sh)

cpu_add_runner(hisya.sh)

mayah_add_test(decision_planner_test)
mayah_add_test(evaluator_test)
mayah_add_test(evaluation_parameter_test)
mayah_add_test(gazer_test)
mayah_add_test(mayah_ai_test)
mayah_add_test(mayah_ai_situation_test)
mayah_add_test(pattern_rensa_detector_test)
mayah_add_test(rensa_hand_tree_test)
mayah_add_test(score_collector_test)
mayah_add_test(shape_evaluator_test)

mayah_add_test(mayah_ai_performance_test 1)
mayah_add_test(gazer_performance_test 1)
mayah_add_test(rensa_hand_tree_performance_test 1)
