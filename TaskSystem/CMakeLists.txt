cmake_minimum_required(VERSION 3.10.0)

project(TaskSystem)

add_executable(TaskSystem main.cpp "Task.h" "TaskSystem.h" "TaskSystem.cpp" "TaskQueue.h" "TaskQueue.cpp" )



target_include_directories(TaskSystem PRIVATE . "../Profile/")