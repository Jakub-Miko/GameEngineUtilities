cmake_minimum_required(VERSION 3.10.0)
project(RuntimeTypeIdentifier)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "./include/")