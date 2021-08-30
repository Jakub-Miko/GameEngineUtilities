cmake_minimum_required(VERSION 3.10.0)
project(Test)
add_executable(${PROJECT_NAME} "test.cpp" "TypeId.h" "test (2).cpp" "Second.h")
target_include_directories(${PROJECT_NAME} PRIVATE .)