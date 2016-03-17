cmake_minimum_required(VERSION 2.8)

add_library(puyoai_solver
            endless.cc
            problem.cc
            puyop.cc
            solver.cc)

add_executable(nazopuyo nazopuyo.cc)
# libraries should be sorted by dependency.
target_link_libraries(nazopuyo puyoai_solver)
target_link_libraries(nazopuyo puyoai_core_pattern)
target_link_libraries(nazopuyo puyoai_core_plan)
target_link_libraries(nazopuyo puyoai_core_rensa_tracker)
target_link_libraries(nazopuyo puyoai_core_rensa)
target_link_libraries(nazopuyo puyoai_core)
target_link_libraries(nazopuyo puyoai_base)
puyoai_target_link_libraries(nazopuyo)
