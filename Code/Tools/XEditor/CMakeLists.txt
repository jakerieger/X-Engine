project(SpaceGame)

set(SCENE_EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/XEditor)

add_executable(XEditor WIN32
    ${IMGUI_SOURCES}
    ${COMMON_SOURCES}
    ${SCENE_EDITOR_DIR}/XEditor.hpp
    ${SCENE_EDITOR_DIR}/XEditor.cpp
    ${SCENE_EDITOR_DIR}/main.cpp
)

target_link_libraries(XEditor
    x
    yaml-cpp
)

target_compile_definitions(XEditor PRIVATE ${WINDOWS_COMPILE_DEFS})