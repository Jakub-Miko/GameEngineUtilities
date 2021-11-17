cmake_minimum_required(VERSION 3.10.0)

project(TaskSystem)

set(CMAKE_CXX_STANDARD 17)
add_library(TaskSystem "Task.h" "TaskSystem.h" "TaskSystemFence.cpp" "TaskSystem.cpp" "TaskQueue.h" "TaskQueue.cpp"  "Promise.h" "Promise.cpp")


find_package(Threads)
target_include_directories(TaskSystem PUBLIC .)
target_link_libraries(TaskSystem PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(TaskSystem PUBLIC Profiler MemoryManagement ThreadManager)