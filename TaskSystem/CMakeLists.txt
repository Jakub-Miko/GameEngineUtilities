cmake_minimum_required(VERSION 3.10.0)

project(TaskSystem)

set(CMAKE_CXX_STANDARD 17)
add_library(TaskSystem "Task.h" "TaskSystem.h" "TaskSystem.cpp" "TaskQueue.h" "TaskQueue.cpp" )


find_package(Threads)
target_include_directories(TaskSystem PUBLIC .)
target_link_libraries(TaskSystem PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(TaskSystem PUBLIC Profiler MemoryManagement)