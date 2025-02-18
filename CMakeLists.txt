cmake_minimum_required(VERSION 3.14)
project(SpaceGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/FetchDeps.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(WINDOWS_COMPILE_DEFS
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

add_executable(TestProgram TestProgram.cpp)
target_link_libraries(TestProgram PRIVATE
    luajit
    sol2
    assimp
    yaml-cpp
    DirectXTex
)

if(WIN32)
    target_compile_definitions(TestProgram PRIVATE ${WINDOWS_COMPILE_DEFS} LUA_JIT)
else()
    message(FATAL_ERROR "Project only supports Windows. Please use Windows.")
endif()