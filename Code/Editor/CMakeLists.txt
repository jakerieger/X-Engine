project(SpaceGame)

set(EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/XEditor)

add_executable(xeditor
    ${IMGUI_SOURCES}
    ${STB_SOURCES}
    ${COMMON_SOURCES}
    ${EDITOR_DIR}/FileDialogs.cpp
    ${EDITOR_DIR}/FileDialogs.hpp
    ${EDITOR_DIR}/TextureManager.cpp
    ${EDITOR_DIR}/TextureManager.hpp
    ${EDITOR_DIR}/EditorWindow.cpp
    ${EDITOR_DIR}/EditorWindow.hpp
    ${EDITOR_DIR}/main.cpp
)

target_link_libraries(xeditor x)
target_compile_definitions(xeditor PRIVATE ${WINDOWS_COMPILE_DEFS} LUA_JIT)