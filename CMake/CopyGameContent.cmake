add_custom_command(
    TARGET SpaceGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Game/Content
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Content
)

add_custom_command(
    TARGET SpaceGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Game/Scripts
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Scripts
)