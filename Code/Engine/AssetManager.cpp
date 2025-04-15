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
            const auto fullPath         = mWorkingDirectory / "Content" / assetFile.Str();
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

    vector<AssetDescriptor> AssetManager::GetAssetDescriptors() {
        vector<AssetDescriptor> assetDescriptors;
#ifdef X_USE_PAK_FILE
        throw std::exception("AssetManager::GetAssetDescriptors - Not available when using pak files");
#else
        for (const auto& [id, filename] : mAssets) {
            assetDescriptors.emplace_back(AssetDescriptor {.mId = id, .mFilename = filename.Str()});
        }
#endif
        return assetDescriptors;
    }

    void AssetManager::ReloadAssets() {
        LoadAssets(mWorkingDirectory);
    }

    bool AssetManager::LoadAssets(const Path& workingDir) {
        mWorkingDirectory = workingDir;
#ifdef X_USE_PAK_FILE
        const auto pakFile = Path(X_PAK_FILE);
        if (!pakFile.Exists()) {
            X_LOG_ERROR("AssetManager::LoadAssets - Pak file not found");
            return false;
        }

        mAssets = XPak::ReadPakTable(pakFile);
#else
        const auto contentDir = workingDir / "Content";
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
                X_LOG_INFO("Loaded asset '%s'", descriptor.mFilename.c_str());
            }
        }
#endif

        mLoaded = true;
        X_LOG_INFO("AssetManager::LoadAssets - Loaded");
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