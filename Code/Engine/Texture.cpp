#include "Texture.hpp"
#include "Common/Str.hpp"

namespace x {
    TextureHandle<Texture2D> TextureLoader::LoadFromFile2D(const str& filename) {
        auto texture2d = CreateTexture2D();

        TexMetadata metadata;
        ScratchImage scratchImage;

        auto hr = LoadFromDDSFile(AnsiToWide(filename).c_str(), DDS_FLAGS_NONE, &metadata, scratchImage);
        PANIC_IF_FAILED(hr, "Failed to load DDS texture file: %s", filename.c_str())

        hr = CreateShaderResourceView(_renderer.GetDevice(),
                                      scratchImage.GetImages(),
                                      scratchImage.GetImageCount(),
                                      metadata,
                                      &texture2d->_textureView);
        PANIC_IF_FAILED(hr, "Failed to create shader resource view from texture.")

        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD         = 0;
        samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

        hr = _renderer.GetDevice()->CreateSamplerState(&samplerDesc, &texture2d->_samplerState);
        PANIC_IF_FAILED(hr, "Failed to create sampler state for texture.");

        return texture2d;
    }

    TextureHandle<Texture2D> TextureLoader::LoadFromMemory2D(const u8* data, size_t size) {
        auto texture2d = CreateTexture2D();

        TexMetadata metadata;
        ScratchImage scratchImage;

        auto hr = LoadFromDDSMemory(data, size, DDS_FLAGS_NONE, &metadata, scratchImage);
        PANIC_IF_FAILED(hr, "Failed to load DDS texture from memory (%llu bytes).", size);

        hr = CreateShaderResourceView(_renderer.GetDevice(),
                                      scratchImage.GetImages(),
                                      scratchImage.GetImageCount(),
                                      metadata,
                                      &texture2d->_textureView);
        PANIC_IF_FAILED(hr, "Failed to create shader resource view from texture.")

        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD         = 0;
        samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

        hr = _renderer.GetDevice()->CreateSamplerState(&samplerDesc, &texture2d->_samplerState);
        PANIC_IF_FAILED(hr, "Failed to create sampler state for texture.");

        return texture2d;
    }

    void Texture2D::Bind(const u32 slot) const {
        _renderer.GetContext()->PSSetShaderResources(slot, 1, _textureView.GetAddressOf());
        _renderer.GetContext()->PSSetSamplers(slot, 1, _samplerState.GetAddressOf());
    }
}