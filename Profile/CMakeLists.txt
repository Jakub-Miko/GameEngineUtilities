cmake_minimum_required(VERSION 3.10.0)


project(Test)


add_executable(${PROJECT_NAME} "Test_1.cpp" Profiler.h )

target_include_directories(${PROJECT_NAME} PRIVATE .)

