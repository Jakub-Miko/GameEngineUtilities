cmake_minimum_required(VERSION 3.11.0)

project(MemoryManagement)

set(CMAKE_CXX_STANDARD 17)

add_library(MemoryManagement INTERFACE)

target_include_directories(MemoryManagement INTERFACE "./include/")