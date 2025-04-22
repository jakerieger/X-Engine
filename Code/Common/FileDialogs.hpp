// Author: Jake Rieger
// Created: 2/20/2025.
//

#pragma once

#include "Platform.hpp"

/// @brief Namespace containing all platform-specific code (Windows API, etc.)
namespace Platform {
    bool OpenFileDialog(
      HWND owner, const char* initialDir, const char* filter, const char* title, char* outPath, size_t outPathSize);
    bool SaveFileDialog(HWND owner,
                        const char* initialDir,
                        const char* filter,
                        const char* title,
                        const char* defaultExt,
                        char* outPath,
                        size_t outPathSize);
    bool SelectFolderDialog(HWND owner, const char* title, char* outPath, size_t outPathSize);
}  // namespace Platform
