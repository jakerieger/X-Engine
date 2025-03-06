project(SpaceGame)

set(GAME_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SpaceGame)

add_executable(SpaceGame
    ${IMGUI_SOURCES}
    ${STB_SOURCES}
    ${COMMON_SOURCES}
    ${GAME_DIR}/GameWindow.cpp
    ${GAME_DIR}/GameWindow.hpp
    ${GAME_DIR}/main.cpp
)

option(USE_PAK_FILE "Enable the use of pak files" ON)

if (USE_PAK_FILE)
    set(PAK_DEFS
        X_USE_PAK_FILE=1
        X_PAK_FILE="Data.xpak"
    )

    target_compile_definitions(x PUBLIC ${PAK_DEFS})
else ()
    include(${CMAKE_CURRENT_SOURCE_DIR}/CopyGameContent.cmake)
endif ()

message("-- Using pak file: ${USE_PAK_FILE}")

target_link_libraries(SpaceGame PRIVATE x)
target_compile_definitions(SpaceGame PRIVATE ${WINDOWS_COMPILE_DEFS} LUA_JIT)