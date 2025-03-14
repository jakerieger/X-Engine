// Author: Jake Rieger
// Created: 3/14/2025.
//

#pragma once

#include <string>
#include "Filesystem.hpp"
#include "Platform.hpp"

namespace Platform {
    using PlatformDirectoryId                            = uint8_t;
    static constexpr PlatformDirectoryId kProgramFiles   = 0;
    static constexpr PlatformDirectoryId kProgramFileX86 = 1;
    static constexpr PlatformDirectoryId kUserHome       = 2;
    static constexpr PlatformDirectoryId kDocuments      = 3;
    static constexpr PlatformDirectoryId kDesktop        = 4;
    static constexpr PlatformDirectoryId kDownloads      = 5;
    static constexpr PlatformDirectoryId kAppDataLocal   = 6;
    static constexpr PlatformDirectoryId kAppDataRoaming = 7;

    std::string GetEnvVar(const std::string& name);
    x::Path GetPlatformDirectory(const PlatformDirectoryId id);
};  // namespace Platform
