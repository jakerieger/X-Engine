// Author: Jake Rieger
// Created: 2/20/2025.
//

#include "FileDialogs.hpp"

namespace x {
    static void PrepareFilterString(char* filter, size_t length) {
        bool lastWasNull = false;
        for (size_t i = 0; i < length; i++) {
            if (filter[i] == '|') filter[i] = '\0';
            if (filter[i] == '\0') {
                if (lastWasNull) break;
                lastWasNull = true;
            } else {
                lastWasNull = false;
            }
        }
        filter[length - 2] = '\0';
        filter[length - 1] = '\0';
    }

    bool OpenFileDialog(
      HWND owner, const char* initialDir, const char* filter, const char* title, char* outPath, size_t outPathSize) {
        OPENFILENAMEA ofn = {0};
        ofn.lStructSize   = sizeof(OPENFILENAMEA);
        ofn.hwndOwner     = owner;

        ofn.lpstrFile = outPath;
        ofn.nMaxFile  = (DWORD)outPathSize;
        *outPath      = '\0';

        char filterCopy[1024] = {0};
        if (filter) {
            strncpy(filterCopy, filter, sizeof(filterCopy) - 1);
            PrepareFilterString(filterCopy, sizeof(filterCopy) - 1);
            ofn.lpstrFilter = filterCopy;
        }

        ofn.lpstrInitialDir = initialDir;
        ofn.lpstrTitle      = title;
        ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

        return ::GetOpenFileNameA(&ofn) != 0;
    }

    bool SaveFileDialog(HWND owner,
                        const char* initialDir,
                        const char* filter,
                        const char* title,
                        const char* defaultExt,
                        char* outPath,
                        size_t outPathSize) {
        OPENFILENAMEA ofn {0};
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner   = owner;

        ofn.lpstrFile = outPath;
        ofn.nMaxFile  = (DWORD)outPathSize;
        *outPath      = '\0';

        char filterCopy[1024] {0};
        if (filter) {
            strncpy(filterCopy, filter, sizeof(filterCopy) - 1);
            PrepareFilterString(filterCopy, sizeof(filterCopy) - 1);
            ofn.lpstrFilter = filterCopy;
        }

        ofn.lpstrInitialDir = initialDir;
        ofn.lpstrTitle      = title;
        ofn.lpstrDefExt     = defaultExt;
        ofn.Flags           = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

        return ::GetSaveFileNameA(&ofn) != 0;
    }

    int ShowAlert(HWND owner, const char* title, const char* message, AlertSeverity severity) {
        UINT type = MB_OK;

        switch (severity) {
            case AlertSeverity::Info:
                type = MB_OK | MB_ICONINFORMATION;
                break;
            case AlertSeverity::Warning:
                type = MB_OK | MB_ICONWARNING;
                break;
            case AlertSeverity::Error:
                type = MB_OK | MB_ICONERROR;
                break;
            case AlertSeverity::Question:
                type = MB_YESNO | MB_ICONQUESTION;
                break;
        }

        return ::MessageBoxA(owner, message, title, type);
    }
}  // namespace x
