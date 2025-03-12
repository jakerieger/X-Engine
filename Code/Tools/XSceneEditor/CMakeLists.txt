project(SpaceGame)

set(SCENE_EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/XSceneEditor)

add_executable(XSceneEditor
    ${IMGUI_SOURCES}
    ${COMMON_SOURCES}
    ${TOOLS_SHARED_SOURCES}
    ${SCENE_EDITOR_DIR}/SceneEditor.hpp
    ${SCENE_EDITOR_DIR}/SceneEditor.cpp
    ${SCENE_EDITOR_DIR}/main.cpp
)

target_link_libraries(XSceneEditor
    x
    yaml-cpp
)

target_compile_definitions(XSceneEditor PRIVATE ${WINDOWS_COMPILE_DEFS})