project(SpaceGame)

set(EDITOR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/XEditor)

set(APP_RES
    ${EDITOR_DIR}/Res/resource.h
    ${EDITOR_DIR}/Res/app.rc
)

add_executable(XEditor WIN32
    ${APP_RES}
    ${COMMON_SOURCES}
    ${IMGUI_SOURCES}
    ${CODE_DIR}/Tools/XPak/Compression.hpp
    ${CODE_DIR}/Tools/XPak/Compression.cpp
    ${EDITOR_DIR}/AssetBrowserIcons.h
    ${EDITOR_DIR}/Controls.cpp
    ${EDITOR_DIR}/Controls.hpp
    ${EDITOR_DIR}/ImGuiHelpers.hpp
    ${EDITOR_DIR}/MeshPreviewer.cpp
    ${EDITOR_DIR}/MeshPreviewer.hpp
    ${EDITOR_DIR}/TextureManager.cpp
    ${EDITOR_DIR}/TextureManager.hpp
    ${EDITOR_DIR}/ToolbarIcons.h
    ${EDITOR_DIR}/WelcomeScreenIcons.h
    ${EDITOR_DIR}/XEditor.cpp
    ${EDITOR_DIR}/XEditor.hpp
    ${EDITOR_DIR}/main.cpp
)

target_link_libraries(XEditor
    x
    yaml-cpp
    ${BROTLI_LIBS}
)

target_compile_definitions(XEditor PRIVATE ${WINDOWS_COMPILE_DEFS})

# Copy editor themes folder
add_custom_command(
    TARGET XEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Themes
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Themes
)

# Copy engine content
add_custom_command(
    TARGET XEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Engine/EngineContent
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/EngineContent
)

# Copy LICENSES.txt
add_custom_command(
    TARGET XEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/LICENSES.txt
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/LICENSES.txt
)