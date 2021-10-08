cmake_minimum_required(VERSION 3.11.0)

project(MemoryManagement)

set(CMAKE_CXX_STANDARD 17)

add_executable(MemoryManagement main.cpp  "MemoryPoolDynamic.h" "SynchronizedMultiPool.h" "MultiPool.h")

target_include_directories(MemoryManagement PRIVATE . "../Profile")
