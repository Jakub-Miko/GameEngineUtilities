cmake_minimum_required(VERSION 3.10.0)
project(FileManager)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/out")


add_library(FileManager "FileManager.h" "FileManager.cpp")

target_include_directories(FileManager PRIVATE .)