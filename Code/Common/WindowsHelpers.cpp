// Author: Jake Rieger
// Created: 3/14/2025.
//

#include "Str.hpp"
#include "WindowsHelpers.hpp"
#include <ShlObj.h>
#include <KnownFolders.h>

namespace Platform {
    static std::string GetKnownFolderPath(const KNOWNFOLDERID& folderId) {
        PWSTR path = nullptr;
        std::string result;
        const HRESULT hr = SHGetKnownFolderPath(folderId, 0, nullptr, &path);
        if (SUCCEEDED(hr)) {
            result = WideToAnsi(path);
            CoTaskMemFree(path);  // Free the memory allocated by SHGetKnownFolderPath
        }
        return result;
    }

    std::string GetEnvVar(const std::string& name) {
        const DWORD bufferSize = ::GetEnvironmentVariableA(name.c_str(), nullptr, 0);
        if (bufferSize == 0) { return ""; }
        std::string result(bufferSize - 1, '\0');
        ::GetEnvironmentVariableA(name.c_str(), &result[0], bufferSize);
        return result;
    }

    x::Path GetPlatformDirectory(const PlatformDirectoryId id) {
        switch (id) {
            case kProgramFiles:
                return x::Path(GetKnownFolderPath(FOLDERID_ProgramFiles));
            case kProgramFileX86:
                return x::Path(GetKnownFolderPath(FOLDERID_ProgramFilesX86));
            case kUserHome:
                return x::Path(GetKnownFolderPath(FOLDERID_UserProfiles));
            case kDocuments:
                return x::Path(GetKnownFolderPath(FOLDERID_Documents));
            case kDesktop:
                return x::Path(GetKnownFolderPath(FOLDERID_Desktop));
            case kDownloads:
                return x::Path(GetKnownFolderPath(FOLDERID_Downloads));
            case kAppDataLocal:
                return x::Path(GetKnownFolderPath(FOLDERID_LocalAppData));
            case kAppDataRoaming:
                return x::Path(GetKnownFolderPath(FOLDERID_RoamingAppData));
            default:
                return x::Path::Current();
        }
    }
}  // namespace Platform