// Author: Jake Rieger
// Created: 3/14/2025.
//

#pragma once

#include "Common/Typedefs.hpp"
#include "Engine/D3D.hpp"
#include "Engine/RenderContext.hpp"
#include <imgui.h>

namespace x {
    struct TextureData {
        ComPtr<ID3D11ShaderResourceView> mShaderResourceView;
        u32 mWidth;
        u32 mHeight;
    };

    /// @brief Simple texture manager class for handling textures used by ImGui
    class TextureManager {
    public:
        explicit TextureManager(RenderContext& context) : mContext(context) {}

        bool LoadFromMemory(const u8* data, u32 width, u32 height, u32 channels, const str& name);
        bool LoadFromMemoryCompressed(
          const u8* data, const size_t compressedSize, u32 width, u32 height, u32 channels, const str& name);
        bool LoadFromDDSFile(const Path& ddsFile, const str& name);

        std::optional<TextureData> GetTexture(const str& name);
        ImTextureID GetTextureID(const str& name);

    private:
        RenderContext& mContext;
        unordered_map<str, TextureData> mTextures;
    };
}  // namespace x
