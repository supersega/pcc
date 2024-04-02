cmake_minimum_required(VERSION 3.0...3.28.3)

PROJECT(pcc VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_subdirectory(examples)
add_subdirectory(pcc)
enable_testing()
add_subdirectory(test)
