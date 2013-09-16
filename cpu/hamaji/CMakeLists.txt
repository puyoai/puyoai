cmake_minimum_required(VERSION 2.8)

cpu_setup("hamaji")

puyoai_add_cxx_flags("-W -Wall -Wno-deprecated-declarations")

add_library(hamaji_lib
            core.cc ctrl.cc db.cc eval_base.cc eval.cc eval2.cc
            field.cc game.cc mt19937int.cc
            rater.cc ratingstats.cc solo.cc util.cc)

function(hamaji_add_executable exe)
  cpu_add_executable(${exe} ${ARGN})
  cpu_target_link_libraries(${exe} hamaji_lib)
  cpu_target_link_common_libraries(${exe})
endfunction()

function(hamaji_add_test exe)
  hamaji_add_executable(${exe} ${exe}.cc)
  cpu_target_link_libraries(${exe} ${GTEST_BOTH_LIBRARIES})
  if(NOT ARGV1)
    cpu_add_test(${exe})
  endif()
endfunction()

hamaji_add_executable(lps main.cc)
hamaji_add_executable(study study.cc)
hamaji_add_executable(mkdb mkdb.cc)
hamaji_add_executable(matchstat matchstat.cc)

if(GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})

  hamaji_add_test(db_test)
  # TODO(hamaji): Slow!
  hamaji_add_test(field_perf_test 1)
  hamaji_add_test(field_test)
endif()

# TODO(hamaji): Build?
# parse_movie.cc
