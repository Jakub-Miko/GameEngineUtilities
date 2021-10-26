cmake_minimum_required(VERSION 3.10.0)
project(ThreadManager)

add_library(ThreadManager "ThreadManager.cpp")

target_include_directories(ThreadManager PRIVATE ".")

target_include_directories(ThreadManager PUBLIC "./include/")
