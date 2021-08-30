cmake_minimum_required(VERSION 3.10.0)
project(LuaTest)
set(CMAKE_CXX_STANDARD 17)

add_executable(LuaTest "test.cpp" "LuaEngine.h" "LuaEngine.cpp")

add_subdirectory(dependencies)

target_compile_definitions(LuaTest PRIVATE DISABLE_ASSERTS)

target_link_libraries(LuaTest PRIVATE lua)

target_include_directories(LuaTest PRIVATE . ../)



