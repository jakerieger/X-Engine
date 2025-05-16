// Author: Jake Rieger
// Created: 3/5/2025.
//

#pragma once

#include "EngineCommon.hpp"
#include "Common/Typedefs.hpp"
#include "Tools/XPak/AssetDescriptor.hpp"

#ifdef X_USE_PAK_FILE
    #ifndef X_PAK_FILE
        #error "X_USE_PAK_FILE is defined, but X_PAK_FILE is not. You nust define the pak file to use."
    #endif

    #include "Tools/XPak/XPak.hpp"
#else
#endif

namespace x {
    class AssetManager {
        friend class Game;
        friend class XEditor;

        X_CLASS_PREVENT_MOVES_COPIES(AssetManager)
        AssetManager() = default;

    public:
        static optional<vector<u8>> GetAssetData(AssetId id);
        static vector<AssetDescriptor> GetAssetDescriptors();
        static void ReloadAssets();

    private:
        inline static bool mLoaded {false};
        static bool LoadAssets(const Path& workingDir = Path::Current());
        static vector<AssetId> GetScenes();

        inline static Path mWorkingDirectory;

#ifdef X_USE_PAK_FILE
        inline static AssetTable mAssets;
#else
        inline static unordered_map<AssetId, Path> mAssets;
#endif
    };
}  // namespace x
