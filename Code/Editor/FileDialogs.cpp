// Author: Jake Rieger
// Created: 2/20/2025.
//

#include "FileDialogs.hpp"

namespace x::Editor {
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
        ofn.nMaxFile  = outPathSize;
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
        return false;
    }
}  // namespace x::Editor
