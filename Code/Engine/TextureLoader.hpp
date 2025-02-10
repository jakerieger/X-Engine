#pragma once

#include "ResourceManager.hpp"
#include "Texture.hpp"
#include "Common/Str.hpp"
#include <DirectXTex.h>

namespace x {
    class TextureLoader2D final : public ResourceLoader<Texture2D> {
        Texture2D LoadImpl(RenderContext& context, const str& path) override {
            Texture2D texture(context);

            TexMetadata metadata;
            ScratchImage scratchImage;

            auto hr = LoadFromDDSFile(AnsiToWide(path).c_str(), DDS_FLAGS_NONE, &metadata, scratchImage);
            PANIC_IF_FAILED(hr, "Failed to load DDS texture file: %s", path.c_str())

            hr = CreateShaderResourceView(context.GetDevice(),
                                          scratchImage.GetImages(),
                                          scratchImage.GetImageCount(),
                                          metadata,
                                          &texture._textureView);
            PANIC_IF_FAILED(hr, "Failed to create shader resource view from texture.")

            D3D11_SAMPLER_DESC samplerDesc{};
            samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            samplerDesc.MinLOD         = 0;
            samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

            hr = context.GetDevice()->CreateSamplerState(&samplerDesc, &texture._samplerState);
            PANIC_IF_FAILED(hr, "Failed to create sampler state for texture.");

            return texture;
        }
    };

    REGISTER_RESOURCE_LOADER(Texture2D, TextureLoader2D)
}