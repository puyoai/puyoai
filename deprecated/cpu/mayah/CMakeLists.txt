cmake_minimum_required(VERSION 2.8)

cpu_setup("mayah")

puyoai_add_cxx_flags("-Wall -Wextra -std=c++0x")

add_library(mayah_lib
            ai.cc ai_manager.cc basic_field.cc enemy_info.cc
            evaluation_feature.cc evaluation_feature_collector.cc field.cc 
            game.cc plan.cc player_info.cc protocol.cc puyo_possibility.cc rensa_detector.cc
            rensa_result.cc)

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
mayah_add_executable(solver solver.cc)
mayah_add_executable(solo solo.cc)

include_directories(${gtest_SOURCE_DIR}/include
                    ${gtest_SOURCE_DIR})

cpu_add_runner(run.sh)
cpu_add_runner(feature.txt)

mayah_add_test(enemy_info_test)
mayah_add_test(evaluation_feature_test)
mayah_add_test(field_bit_field_test)
mayah_add_test(field_test)
mayah_add_test(puyo_set_test)
mayah_add_test(puyo_test)
mayah_add_test(rensa_detector_test)
# TODO(hamaji): Slow!
mayah_add_test(performance_test 1)
mayah_add_test(puyo_possibility_test 1)

