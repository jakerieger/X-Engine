cmake_minimum_required(VERSION 3.14)
project(SpaceGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/FetchDeps.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Code)

include(${CODE_DIR}/Common.cmake)
include(${CODE_DIR}/Engine.cmake)
include(${CODE_DIR}/Game.cmake)
include(${CODE_DIR}/Vendor.cmake)

include_directories(
    ${IMGUI_INCLUDE}
    ${SHADER_INCLUDE}
    ${VENDOR_INCLUDE}
    ${CODE_DIR}
)

add_library(x STATIC
    ${IMGUI_SOURCES}
    ${STB_SOURCES}
    ${COMMON_SOURCES}
    ${ENGINE_SOURCES}
)

add_executable(SpaceGame
    ${COMMON_SOURCES}
    ${GAME_SOURCES}
)

if(WIN32)
    set(WINDOWS_COMPILE_DEFS
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    set(DX_LIBS
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
        dxguid.lib
    )

    target_link_libraries(x PUBLIC
        ${DX_LIBS}
        luajit
        sol2
        assimp
        yaml-cpp
        DirectXTex
    )
    target_compile_definitions(x PRIVATE ${WINDOWS_COMPILE_DEFS} LUA_JIT)

    target_link_libraries(SpaceGame PRIVATE x)
    target_compile_definitions(SpaceGame PRIVATE ${WINDOWS_COMPILE_DEFS} LUA_JIT)
else()
    message(FATAL_ERROR "Project only supports Windows 10 and 11.")
endif()