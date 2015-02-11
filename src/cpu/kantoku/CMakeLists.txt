cmake_minimum_required(VERSION 2.8)

add_executable(kantoku main.cc)

puyoai_target_link_libraries(kantoku)
target_link_libraries(kantoku puyoai_core_client_ai)
target_link_libraries(kantoku puyoai_core_client_connector)
target_link_libraries(kantoku puyoai_core_server_connector)
target_link_libraries(kantoku puyoai_core)
target_link_libraries(kantoku puyoai_base)

cpu_add_runner(run.sh)
