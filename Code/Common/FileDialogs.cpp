// Author: Jake Rieger
// Created: 2/20/2025.
//

#include "FileDialogs.hpp"
#include <shobjidl.h>
#include <shlobj.h>
#include <combaseapi.h>
#include <string>
#include <vector>

namespace Platform {
    static std::vector<std::wstring> SplitFilterString(const char* filter) {
        std::vector<std::wstring> result;
        const char* p = filter;
        while (*p) {
            std::string part;
            while (*p && *p != '|')
                part += *p++;
            if (*p == '|') ++p;
            result.push_back(std::wstring(part.begin(), part.end()));
        }
        return result;
    }

    static void SetDialogDirectory(IFileDialog* dialog, const char* initialDir) {
        if (!initialDir || !*initialDir) return;

        IShellItem* folder {nullptr};
        const HRESULT hr =
          SHCreateItemFromParsingName(std::wstring(initialDir, initialDir + strlen(initialDir)).c_str(),
                                      nullptr,
                                      IID_PPV_ARGS(&folder));
        if (SUCCEEDED(hr)) {
            std::ignore = dialog->SetFolder(folder);
            folder->Release();
        }
    }

    static bool ShowFileDialogCommon(HWND owner,
                                     CLSID dialogCLSID,
                                     DWORD options,
                                     const char* initialDir,
                                     const char* filter,
                                     const char* title,
                                     char* outPath,
                                     size_t outPathSize,
                                     bool isSave,
                                     const char* defaultExt) {
        IFileDialog* pDialog = nullptr;
        const auto hr        = CoCreateInstance(dialogCLSID, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));
        if (FAILED(hr)) {
            CoUninitialize();
            return false;
        }

        DWORD currentOptions;
        pDialog->GetOptions(&currentOptions);
        pDialog->SetOptions(currentOptions | options);

        if (title) { pDialog->SetTitle(std::wstring(title, title + strlen(title)).c_str()); }

        SetDialogDirectory(pDialog, initialDir);

        if (isSave && defaultExt) {
            pDialog->SetDefaultExtension(std::wstring(defaultExt, defaultExt + strlen(defaultExt)).c_str());
        }

        if (filter) {
            auto parts = SplitFilterString(filter);
            std::vector<COMDLG_FILTERSPEC> specs;
            std::vector<std::wstring> wideStrings;

            for (size_t i = 0; i + 1 < parts.size(); i += 2) {
                wideStrings.push_back(parts[i]);
                wideStrings.push_back(parts[i + 1]);

                COMDLG_FILTERSPEC spec;
                spec.pszName = wideStrings[wideStrings.size() - 2].c_str();
                spec.pszSpec = wideStrings[wideStrings.size() - 1].c_str();
                specs.push_back(spec);
            }

            if (!specs.empty()) {
                pDialog->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
                pDialog->SetFileTypeIndex(1);  // Select first by default
            }
        }

        bool result = false;
        if (SUCCEEDED(pDialog->Show(owner))) {
            IShellItem* pItem = nullptr;
            if (SUCCEEDED(pDialog->GetResult(&pItem))) {
                PWSTR pszFilePath = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                    size_t len   = wcstombs(outPath, pszFilePath, outPathSize - 1);
                    outPath[len] = '\0';
                    CoTaskMemFree(pszFilePath);
                    result = true;
                }
                pItem->Release();
            }
        }

        pDialog->Release();
        return result;
    }

    bool OpenFileDialog(
      HWND owner, const char* initialDir, const char* filter, const char* title, char* outPath, size_t outPathSize) {
        return ShowFileDialogCommon(owner,
                                    CLSID_FileOpenDialog,
                                    FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST,
                                    initialDir,
                                    filter,
                                    title,
                                    outPath,
                                    outPathSize,
                                    false,
                                    nullptr);
    }

    bool SaveFileDialog(HWND owner,
                        const char* initialDir,
                        const char* filter,
                        const char* title,
                        const char* defaultExt,
                        char* outPath,
                        size_t outPathSize) {
        return ShowFileDialogCommon(owner,
                                    CLSID_FileSaveDialog,
                                    FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT,
                                    initialDir,
                                    filter,
                                    title,
                                    outPath,
                                    outPathSize,
                                    true,
                                    defaultExt);
    }

    bool SelectFolderDialog(HWND owner, const char* title, char* outPath, size_t outPathSize) {
        IFileDialog* pDialog = nullptr;
        const auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));
        if (FAILED(hr)) {
            CoUninitialize();
            return false;
        }

        DWORD options;
        pDialog->GetOptions(&options);
        pDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

        if (title) { pDialog->SetTitle(std::wstring(title, title + strlen(title)).c_str()); }

        bool result = false;
        if (SUCCEEDED(pDialog->Show(owner))) {
            IShellItem* pItem = nullptr;
            if (SUCCEEDED(pDialog->GetResult(&pItem))) {
                PWSTR pszFolderPath = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath))) {
                    size_t len   = wcstombs(outPath, pszFolderPath, outPathSize - 1);
                    outPath[len] = '\0';
                    CoTaskMemFree(pszFolderPath);
                    result = true;
                }
                pItem->Release();
            }
        }

        pDialog->Release();
        return result;
    }
}  // namespace Platform
