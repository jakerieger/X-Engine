project(SpaceGame)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Tools)

add_executable(img2c
    ${CODE_DIR}/Vendor/stb_image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/img2c.c
)