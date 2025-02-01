#pragma once

#include "D3D.hpp"
#include "Renderer.hpp"
#include <DirectXTex.h>

namespace x {
    class Texture1D;
    class Texture2D;
    class Texture3D;
    class TextureCubemap;
    class TextureLoader;

    template<typename T>
    concept IsTexture = std::is_same_v<T, Texture1D> || std::is_same_v<T, Texture2D> || std::is_same_v<T, Texture3D> ||
                        std::is_same_v<T, TextureCubemap>;

    template<typename T>
        requires IsTexture<T>
    using TextureHandle = shared_ptr<T>;

    class Texture2D {
        friend TextureLoader;

        ComPtr<ID3D11ShaderResourceView> _textureView;
        ComPtr<ID3D11Resource> _textureResource;
        ComPtr<ID3D11SamplerState> _samplerState;
        Renderer& _renderer;

        explicit Texture2D(Renderer& renderer) : _renderer(renderer) {}

    public:
        void Bind(u32 slot) const;
    };

    class TextureLoader {
        Renderer& _renderer;

        TextureHandle<Texture2D> CreateTexture2D() {
            return TextureHandle<Texture2D>(new Texture2D(_renderer));
        }

    public:
        explicit TextureLoader(Renderer& renderer) : _renderer(renderer) {}

        TextureHandle<Texture2D> LoadFromFile2D(const str& filename);
        TextureHandle<Texture2D> LoadFromMemory2D(const u8* data, size_t size);
    };
}