// Author: Jake Rieger
// Created: 3/5/2025.
//

#pragma once

#include "EngineCommon.hpp"
#include "Common/Types.hpp"
#include "XPak/AssetDescriptor.hpp"

#ifdef X_USE_PAK_FILE
    #ifndef X_PAK_FILE
        #error "X_USE_PAK_FILE is defined, but X_PAK_FILE is not. You nust define the pak file to use."
    #endif

    #include "XPak/XPak.hpp"
#else
#endif

namespace x {
    class AssetManager {
        friend class Game;
        X_CLASS_PREVENT_MOVES_COPIES(AssetManager)
        AssetManager() = default;

    public:
        static optional<vector<u8>> GetAssetData(AssetId id);
        static vector<AssetDescriptor> GetAssetDescriptors();

    private:
        inline static bool mLoaded {false};
        static bool LoadAssets(const Path& workingDir = Path::Current());
        static vector<AssetId> GetScenes();

#ifdef X_USE_PAK_FILE
        inline static AssetTable mAssets;
#else
        inline static unordered_map<AssetId, Path> mAssets;
#endif
    };
}  // namespace x
