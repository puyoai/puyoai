cmake_minimum_required(VERSION 2.8)

add_library(puyoai_base
            executor.cc flags.cc file.cc time.cc time_stamp_counter.cc strings.cc wait_group.cc)
