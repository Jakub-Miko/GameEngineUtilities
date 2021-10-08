cmake_minimum_required(VERSION 3.10.0)

project(TaskSystem)

set(CMAKE_CXX_STANDARD 17)
add_executable(TaskSystem main.cpp "Task.h" "TaskSystem.h" "TaskSystem.cpp" "TaskQueue.h" "TaskQueue.cpp" )


find_package(Threads)
target_include_directories(TaskSystem PRIVATE . "../Profile/" "../MemoryManagement/")
target_link_libraries(TaskSystem PRIVATE ${CMAKE_THREAD_LIBS_INIT})