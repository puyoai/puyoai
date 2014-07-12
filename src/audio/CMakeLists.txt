cmake_minimum_required(VERSION 2.8)

add_library(puyoai_audio
            audio_commentator.cc)

function(audio_add_executable exe)
    add_executable(${exe} ${ARGN})
    if(USE_INTERNAL AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        target_link_libraries(${exe} puyoai_internal_audio_darwin)
        target_link_libraries(${exe} ${COCOA_LIBRARY})
    endif()
    target_link_libraries(${exe} puyoai_audio)
    target_link_libraries(${exe} puyoai_core)
    target_link_libraries(${exe} puyoai_base)
    puyoai_target_link_libraries(${exe})
endfunction()

audio_add_executable(audio_tester audio_tester.cc)

