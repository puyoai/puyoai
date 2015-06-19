cmake_minimum_required(VERSION 2.8)

function(tool_add_executable exe)
    add_executable(${exe} ${ARGN})
    target_link_libraries(${exe} puyoai_gui)
    target_link_libraries(${exe} puyoai_recognition)
    target_link_libraries(${exe} puyoai_core)
    target_link_libraries(${exe} puyoai_base)
    target_link_libraries(${exe} ${SDL2_LIBRARIES})
    target_link_libraries(${exe} ${SDL2_TTF_LIBRARIES})
    target_link_libraries(${exe} ${SDL2_IMAGE_LIBRARIES})
    puyoai_target_link_libraries(${exe})
endfunction()

tool_add_executable(exhaustive_test_generator exhaustive_test_generator.cc)

tool_add_executable(arow arow.cc)
