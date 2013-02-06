cmake_minimum_required(VERSION 2.8)

function(cpu_setup namespace)
  include_directories(../..)
  set(CPU_NAMESPACE ${namespace} PARENT_SCOPE)
endfunction()

function(cpu_target_link_common_libraries target)
  target_link_libraries(${target}_${CPU_NAMESPACE} cpu_lib)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_util)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core)
  puyoai_target_link_libraries(${target}_${CPU_NAMESPACE})
endfunction()

function(cpu_target_link_libraries target)
  target_link_libraries(${target}_${CPU_NAMESPACE} ${ARGN})
endfunction()

function(cpu_add_executable exe)
  add_executable(${exe}_${CPU_NAMESPACE} ${ARGN})
  set_target_properties(${exe}_${CPU_NAMESPACE} PROPERTIES OUTPUT_NAME ${exe})
endfunction()

function(cpu_add_test exe)
  add_test(check-${exe}_${CPU_NAMESPACE} ${exe})
endfunction()

add_subdirectory(hamaji)
add_subdirectory(hidehiko)
add_subdirectory(hiroshimizuno)
add_subdirectory(ichikawa)
add_subdirectory(pascal)
add_subdirectory(peria)
add_subdirectory(sample)
add_subdirectory(shinyak)
add_subdirectory(yhayashi)

include_directories(..)
add_library(cpu_lib cpu_interface.cc)
