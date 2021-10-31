cmake_minimum_required(VERSION 3.10.0)

project(ConfigManager)

add_library(ConfigManager  "include/ConfigManager.h" "ConfigManager.cpp")

target_include_directories(ConfigManager PRIVATE .)
target_include_directories(ConfigManager PUBLIC "./include/")