// Author: Jake Rieger
// Created: 2/20/2025.
//

#pragma once

#include "Engine/Platform.hpp"

namespace x::Editor {
    bool OpenFileDialog(
      HWND owner, const char* initialDir, const char* filter, const char* title, char* outPath, size_t outPathSize);

    bool SaveFileDialog(HWND owner,
                        const char* initialDir,
                        const char* filter,
                        const char* title,
                        const char* defaultExt,
                        char* outPath,
                        size_t outPathSize);
}  // namespace x::Editor
