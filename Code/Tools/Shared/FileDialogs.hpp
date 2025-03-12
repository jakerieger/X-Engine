// Author: Jake Rieger
// Created: 2/20/2025.
//

#pragma once

#include "Engine/Platform.hpp"

namespace x {
    bool OpenFileDialog(
      HWND owner, const char* initialDir, const char* filter, const char* title, char* outPath, size_t outPathSize);

    bool SaveFileDialog(HWND owner,
                        const char* initialDir,
                        const char* filter,
                        const char* title,
                        const char* defaultExt,
                        char* outPath,
                        size_t outPathSize);

    enum class AlertSeverity {
        Info,
        Warning,
        Error,
        Question,
    };

    int ShowAlert(HWND owner, const char* title, const char* message, AlertSeverity severity);
}  // namespace x
