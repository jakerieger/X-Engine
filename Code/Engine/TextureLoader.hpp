#pragma once

#include "ResourceManager.hpp"
#include "Texture.hpp"
#include "Common/Str.hpp"
#include <DirectXTex.h>

#include "AssetManager.hpp"

namespace x {
    class TextureLoader2D final : public ResourceLoader<Texture2D> {
        Texture2D LoadImpl(RenderContext& context, const u64 id) override {
            Texture2D texture(context);

            TexMetadata metadata;
            ScratchImage scratchImage;

            const auto textureBytes = AssetManager::GetAssetData(id);
            if (!textureBytes.has_value()) {
                X_LOG_ERROR("Failed to load texture with id %llu", id);
                return texture;
            }

            auto hr =
              LoadFromDDSMemory(textureBytes->data(), textureBytes->size(), DDS_FLAGS_NONE, &metadata, scratchImage);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to load DDS texture file: %llu", id)

            hr = CreateShaderResourceView(context.GetDevice(),
                                          scratchImage.GetImages(),
                                          scratchImage.GetImageCount(),
                                          metadata,
                                          &texture.mTextureView);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create shader resource view from texture.")

            D3D11_SAMPLER_DESC samplerDesc {};
            samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            samplerDesc.MinLOD         = 0;
            samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

            hr = context.GetDevice()->CreateSamplerState(&samplerDesc, &texture.mSamplerState);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create sampler state for texture.");

            return texture;
        }
    };

    X_REGISTER_RESOURCE_LOADER(Texture2D, TextureLoader2D)
}  // namespace x