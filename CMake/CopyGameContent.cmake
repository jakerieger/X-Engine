add_custom_command(
    TARGET SpaceGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Game/Content
    ${CMAKE_CURRENT_BINARY_DIR}/Content
)