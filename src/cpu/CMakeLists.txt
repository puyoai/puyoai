cmake_minimum_required(VERSION 2.8)

function(cpu_setup namespace)
  set(CPU_NAMESPACE ${namespace} PARENT_SCOPE)
endfunction()

function(cpu_target_link_common_libraries target)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_solver)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_pattern)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_rensa)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_plan)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_probability)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_client_ai)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_client_connector)

  if(USE_HTTPD)
    target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_httpd)
    target_link_libraries(${target}_${CPU_NAMESPACE} ${LIB_MICROHTTPD})
  endif()
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core_rensa_tracker)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_core)
  target_link_libraries(${target}_${CPU_NAMESPACE} puyoai_base)
  target_link_libraries(${target}_${CPU_NAMESPACE} ${LIB_JSONCPP})
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

function(cpu_add_runner runner)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${runner}
       DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
endfunction()

add_subdirectory(hamaji)
add_subdirectory(mayah)
add_subdirectory(munetoshi)
add_subdirectory(peria)
add_subdirectory(ryumas)
add_subdirectory(takapt)
add_subdirectory(test_lockit)
add_subdirectory(tzik)
add_subdirectory(yamaguchi)
add_subdirectory(yuricat)

add_subdirectory(sample)
add_subdirectory(sample_beam)
add_subdirectory(sample_rensa)
add_subdirectory(sample_suicide)

add_subdirectory(control_tester)

add_subdirectory(kantoku)
add_subdirectory(colun)
