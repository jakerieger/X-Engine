// Author: Jake Rieger
// Created: 2/21/2025.
//

#pragma once

#include <imgui.h>
#include "Common/Types.hpp"
#include "Engine/D3D.hpp"
#include "Engine/RenderContext.hpp"

namespace x::Editor {
    struct TextureData {
        ComPtr<ID3D11ShaderResourceView> srv;
        int width;
        int height;
    };

    class TextureManager {
    public:
        explicit TextureManager(RenderContext& context) : _context(context) {}

        bool LoadFromMemory(const u8* data, int width, int height, int channels, const str& name);
        std::optional<TextureData> GetTexture(const str& name);

    private:
        RenderContext& _context;
        unordered_map<str, TextureData> _textures;
    };
}  // namespace x::Editor
