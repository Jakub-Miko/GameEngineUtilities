cmake_minimum_required(VERSION 3.10.0)
project(LuaTest)
set(CMAKE_CXX_STANDARD 17)

add_library(LuaEngine "LuaEngine.cpp" "include/LuaEngineUtilities.h" "LuaEngineUtilities.cpp")

add_subdirectory("dependencies")

target_compile_definitions(LuaEngine PRIVATE DISABLE_ASSERTS)

find_package(Threads)

target_link_libraries(LuaEngine PUBLIC Lua_Jit ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(LuaEngine PUBLIC Profiler RuntimeTypeIdentifier FileManager ConfigManager JsonParser)
target_include_directories(LuaEngine PRIVATE ".")
target_include_directories(LuaEngine PUBLIC "./include/")

