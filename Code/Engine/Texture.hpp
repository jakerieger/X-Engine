#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include <DirectXTex.h>

namespace x {
    class RenderContext;
    class Texture1D;
    class Texture2D;
    class Texture3D;
    class TextureCubemap;
    class TextureLoader;

    template<typename T>
    concept IsTexture = std::is_same_v<T, Texture1D> || std::is_same_v<T, Texture2D> || std::is_same_v<T, Texture3D> ||
                        std::is_same_v<T, TextureCubemap>;

    class Texture2D {
        friend TextureLoader;
        friend class TextureLoader2D;

        ComPtr<ID3D11ShaderResourceView> _textureView;
        ComPtr<ID3D11Resource> _textureResource;
        ComPtr<ID3D11SamplerState> _samplerState;
        RenderContext& _renderer;

        explicit Texture2D(RenderContext& renderer) : _renderer(renderer) {}

    public:
        void Bind(u32 slot) const;
        void Unbind(u32 slot) const;
    };
}