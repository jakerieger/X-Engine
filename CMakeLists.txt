cmake_minimum_required(VERSION 3.14)
project(XENGINE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

include(FetchContent)
include(${CMAKE_DIR}/FetchDeps.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Code)
set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Code/Tools)

include(${CODE_DIR}/Common.cmake)
include(${CODE_DIR}/Vendor.cmake)

include_directories(
    ${IMGUI_INCLUDE}
    ${VENDOR_INCLUDE}
    ${CODE_DIR}
)

if (WIN32)
    set(WINDOWS_COMPILE_DEFS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )

    add_subdirectory(${CODE_DIR}/Engine)
    # Tools
    add_subdirectory(${CODE_DIR}/Tools/ResPak)
    add_subdirectory(${CODE_DIR}/Tools/XEditor)
    add_subdirectory(${CODE_DIR}/Tools/XPak)

    # Installation setup
    set(XENGINE_INSTALL_BINDIR "bin")
    set(XENGINE_INSTALL_LIBDIR "lib")
    set(XENGINE_INSTALL_INCLUDEDIR "include")
    set(XENGINE_INSTALL_CMAKEDIR "cmake/XENGINE")

    include(CMakePackageConfigHelpers)

    # Configure the version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/XENGINEConfigVersion.cmake"
        VERSION 1.0.0  # Set your version here
        COMPATIBILITY SameMajorVersion
    )

    # Configure the config file
    configure_package_config_file(
        "${CMAKE_DIR}/XENGINEConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/XENGINEConfig.cmake"
        INSTALL_DESTINATION ${XENGINE_INSTALL_CMAKEDIR}
    )

    # Install headers for Engine
    install(
        DIRECTORY ${CODE_DIR}/Engine/
        DESTINATION ${XENGINE_INSTALL_INCLUDEDIR}/XENGINE/Engine
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )

    # Install common headers
    install(
        DIRECTORY ${CODE_DIR}/Common/
        DESTINATION ${XENGINE_INSTALL_INCLUDEDIR}/XENGINE/Common
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )

    # Install headers for XPak
    install(
        DIRECTORY ${CODE_DIR}/Tools/XPak/
        DESTINATION ${XENGINE_INSTALL_INCLUDEDIR}/XENGINE/Tools/XPak
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )

    # Install vendor headers
    install(
        DIRECTORY ${CODE_DIR}/Vendor
        DESTINATION ${XENGINE_INSTALL_INCLUDEDIR}/XENGINE/
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )

    # Install both Engine and XPak targets
    install(
        TARGETS x xpak yaml-cpp brotlienc brotlidec brotlicommon assimp DirectXTex sol2 zlibstatic
        EXPORT XENGINETargets
        LIBRARY DESTINATION ${XENGINE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${XENGINE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${XENGINE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${XENGINE_INSTALL_INCLUDEDIR}
    )

    # Export targets for installation
    install(
        EXPORT XENGINETargets
        FILE XENGINETargets.cmake
        NAMESPACE X::
        DESTINATION ${XENGINE_INSTALL_CMAKEDIR}
    )

    # Install config files
    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/XENGINEConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/XENGINEConfigVersion.cmake"
        DESTINATION ${XENGINE_INSTALL_CMAKEDIR}
    )
else ()
    message(FATAL_ERROR "Project only supports Windows.")
endif ()