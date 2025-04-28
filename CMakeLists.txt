cmake_minimum_required(VERSION 3.14)
project(SpaceGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

include(FetchContent)
include(${CMAKE_DIR}/FetchDeps.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Code)
set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Code/Tools)

include(${CODE_DIR}/Common.cmake)
include(${CODE_DIR}/Vendor.cmake)

include_directories(
    ${IMGUI_INCLUDE}
    ${VENDOR_INCLUDE}
    ${CODE_DIR}
)

if (WIN32)
    set(WINDOWS_COMPILE_DEFS
        #        UNICODE
        #        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )

    add_subdirectory(${CODE_DIR}/Engine)
    #    add_subdirectory(${CODE_DIR}/Editor)
    #    add_subdirectory(${CODE_DIR}/Game)

    # Tools
    add_subdirectory(${CODE_DIR}/Tools/ResPak)
    add_subdirectory(${CODE_DIR}/Tools/XEditor)
    add_subdirectory(${CODE_DIR}/Tools/XPak)
else ()
    message(FATAL_ERROR "Project only supports Windows.")
endif ()