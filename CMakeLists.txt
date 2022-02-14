cmake_minimum_required(VERSION 3.19)
project(hw_02)

set(CMAKE_CXX_STANDARD 20)

include_directories(main/include)
include_directories(test/include)

add_executable(hw_02 main/src/main.cpp main/src/huffman.cpp main/include/huffman.h)
add_executable(hw_02_test test/src/test.cpp test/include/doctest.h main/src/huffman.cpp main/include/huffman.h)

target_compile_definitions(hw_02_test PUBLIC DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test/data/")
