project(XENGINE)

set(RESPAK_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ResPak)

add_executable(respak
    ${COMMON_SOURCES}
    ${CODE_DIR}/Tools/XPak/Compression.hpp
    ${CODE_DIR}/Tools/XPak/Compression.cpp
    ${RESPAK_DIR}/main.cpp
)

target_link_libraries(respak PRIVATE
    ${BROTLI_LIBS}
    CLI11
)