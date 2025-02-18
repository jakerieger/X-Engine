FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v5.4.3
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE) # I might end up needing this, TBD
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    luajit
    GIT_REPOSITORY https://github.com/LuaJIT/LuaJIT.git
    GIT_TAG v2.0.5
)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/BuildLuaJIT.cmake)
build_luajit()

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.1
)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG yaml-cpp-0.7.0
)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

if(NOT WIN32)
    message(FATAL_ERROR "DirectXTex requires Windows")
endif()

FetchContent_Declare(
    directxtex
    GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
    GIT_TAG feb2024
)

set(DIRECTXTEX_BUILD_STATIC_LIB ON CACHE BOOL "Build static library" FORCE)
set(DIRECTXTEX_SHARED_LIBS OFF CACHE BOOL "Build shared library" FORCE)
set(BUILD_DX12 OFF CACHE BOOL "Build with DirectX12 support" FORCE)
set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTex command-line tools" FORCE)
set(ENABLE_OPENEXR OFF CACHE BOOL "Enable OpenEXR support" FORCE)

FetchContent_MakeAvailable(
    assimp
    sol2
    yaml-cpp
    directxtex
)