#pragma once

#include "Common/Typedefs.hpp"
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

        ComPtr<ID3D11ShaderResourceView> mTextureView;
        ComPtr<ID3D11Resource> mTextureResource;
        ComPtr<ID3D11SamplerState> mSamplerState;
        RenderContext& mRenderer;

        explicit Texture2D(RenderContext& renderer) : mRenderer(renderer) {}

    public:
        void Bind(u32 slot) const;
        void Unbind(u32 slot) const;
    };
}  // namespace x