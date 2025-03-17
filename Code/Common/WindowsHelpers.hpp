// Author: Jake Rieger
// Created: 3/14/2025.
//

#pragma once

#include <string>
#include "Filesystem.hpp"
#include "Platform.hpp"

namespace Platform {
    using PlatformDirectoryId                                        = uint8_t;
    static constexpr PlatformDirectoryId kPlatformDir_ProgramFiles   = 0;
    static constexpr PlatformDirectoryId kPlatformDir_ProgramFileX86 = 1;
    static constexpr PlatformDirectoryId kPlatformDir_UserHome       = 2;
    static constexpr PlatformDirectoryId kPlatformDir_Documents      = 3;
    static constexpr PlatformDirectoryId kPlatformDir_Desktop        = 4;
    static constexpr PlatformDirectoryId kPlatformDir_Downloads      = 5;
    static constexpr PlatformDirectoryId kPlatformDir_AppDataLocal   = 6;
    static constexpr PlatformDirectoryId kPlatformDir_AppDataRoaming = 7;

    std::string GetEnvVar(const std::string& name);
    x::Path GetPlatformDirectory(const PlatformDirectoryId id);
};  // namespace Platform
