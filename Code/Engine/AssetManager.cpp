// Author: Jake Rieger
// Created: 3/5/2025.
//

#include "AssetManager.hpp"
#include <ranges>

#ifndef X_USE_PAK_FILE
    #include "XPak/ScriptCompiler.hpp"
#endif

namespace x {
    optional<vector<u8>> AssetManager::GetAssetData(AssetId id) {
        using namespace x::Filesystem;

        if (!mLoaded) {
            X_LOG_ERROR("AssetManager::GetAssetData - Not Loaded");
            return std::nullopt;
        }

#ifdef X_USE_PAK_FILE
        if (auto it = mAssets.find(id); it != mAssets.end()) {
            const auto& [id, asset] = *it;
            auto assetData          = XPak::FetchAssetData(Path(X_PAK_FILE), asset);
            return assetData;
        } else {
            X_LOG_ERROR("AssetManager::GetAssetData - Not Found");
            return std::nullopt;
        }
#else
        if (auto it = mAssets.find(id); it != mAssets.end()) {
            const auto& [id, assetFile] = *it;
            const auto fullPath         = Path::Current() / "Content" / assetFile.Str();
            if (!fullPath.Exists()) { X_LOG_ERROR("AssetManager::GetAssetData - Not Found"); }

            const auto type = AssetDescriptor::GetTypeFromId(id);
            if (type == kAssetType_Script) {
                // Compile bytecode and return that
                const auto scriptSource = FileReader::ReadAllText(fullPath);
                vector<u8> bytecode     = ScriptCompiler::Compile(scriptSource, fullPath.Str());
                return bytecode;
            }

            return FileReader::ReadAllBytes(fullPath);
        } else {
            X_LOG_ERROR("AssetManager::GetAssetData - Not Found");
            return std::nullopt;
        }
#endif
    }

    bool AssetManager::LoadAssets() {
        using namespace x::Filesystem;

#ifdef X_USE_PAK_FILE
        const auto pakFile = Path(X_PAK_FILE);
        if (!pakFile.Exists()) {
            X_LOG_ERROR("AssetManager::LoadAssets - Pak file not found");
            return false;
        }

        mAssets = XPak::ReadPakTable(pakFile);
#else
        const auto rootDir    = Path::Current();
        const auto contentDir = rootDir / "Content";
        if (!contentDir.Exists()) {
            X_LOG_ERROR("AssetManager::LoadAssets - Content directory not Found");
            return false;
        }

        // Iterate over xasset files
        for (const Path& file : contentDir.Entries()) {
            if (file.IsFile() && file.HasExtension() && file.Extension() == "xasset") {
                AssetDescriptor descriptor;
                if (!descriptor.FromFile(file.Str())) {
                    X_LOG_ERROR("AssetManager::LoadAssets - Failed to load asset descriptor '%s'", file.CStr());
                    return false;
                }
                mAssets[descriptor.mId] = Path(descriptor.mFilename);
            }
        }
#endif

        mLoaded = true;
        return true;
    }

    vector<AssetId> AssetManager::GetScenes() {
        if (!mLoaded) {
            X_LOG_ERROR("AssetManager::GetScenes - Not Loaded");
            return {};
        }

        vector<AssetId> scenes;
        for (const auto& id : mAssets | std::views::keys) {
            if (AssetDescriptor::GetTypeFromId(id) == kAssetType_Scene) { scenes.emplace_back(id); }
        }
        return scenes;
    }
}  // namespace x