project(SpaceGame)

set(XPAK_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/XPak)

add_library(xpak STATIC
    ${COMMON_SOURCES}
    ${XPAK_DIR}/XPak.hpp
    ${XPAK_DIR}/XPak.cpp
    ${XPAK_DIR}/AssetDescriptor.hpp
    ${XPAK_DIR}/AssetDescriptor.cpp
    ${XPAK_DIR}/ProjectDescriptor.hpp
    ${XPAK_DIR}/ProjectDescriptor.cpp
    ${XPAK_DIR}/Compression.hpp
    ${XPAK_DIR}/Compression.cpp
    ${XPAK_DIR}/AssetGenerator.hpp
    ${XPAK_DIR}/AssetGenerator.cpp
)

add_executable(xpakc
    ${XPAK_DIR}/main.cpp
)

target_link_libraries(xpak PRIVATE
    yaml-cpp
    ${BROTLI_LIBS}
)

target_link_libraries(xpakc
    xpak
    yaml-cpp
    ${BROTLI_LIBS}
    CLI11
)