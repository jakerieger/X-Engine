cmake_minimum_required(VERSION 3.14)
project(SpaceGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/FetchDeps.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(TestProgram TestProgram.cpp)
target_link_libraries(TestProgram PRIVATE
    luajit
    sol2
    assimp
    yaml-cpp
    DirectXTex
)
target_compile_definitions(TestProgram PRIVATE LUA_JIT)