cmake_minimum_required(VERSION 3.10.0)


project(Profiler)

set(PROFILER_BACKEND "CHROME_TRACE" CACHE STRING "CHROME_TRACE or NVTX")

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE "./include/")

add_subdirectory("./dependencies/NVTX/")

target_link_libraries(${PROJECT_NAME} INTERFACE nvtx3-cpp)

target_compile_definitions(${PROJECT_NAME} INTERFACE ${PROFILER_BACKEND})
