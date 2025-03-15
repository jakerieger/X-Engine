// Author: Jake Rieger
// Created: 3/14/2025.
//

#include "TextureManager.hpp"
#include "Common/Str.hpp"
#include <DirectXTex.h>

namespace x {
    bool TextureManager::LoadFromMemory(const u8* data, u32 width, u32 height, u32 channels, const str& name) {
        TextureData texture {};
        texture.mWidth  = width;
        texture.mHeight = height;

        D3D11_TEXTURE2D_DESC desc {};
        desc.Width            = width;
        desc.Height           = height;
        desc.MipLevels        = 1;
        desc.ArraySize        = 1;
        desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage            = D3D11_USAGE_DEFAULT;
        desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA subresource {};
        subresource.pSysMem     = data;
        subresource.SysMemPitch = width * channels;

        ComPtr<ID3D11Texture2D> texture2D;
        auto* device = mContext.GetDevice();
        if (SUCCEEDED(device->CreateTexture2D(&desc, &subresource, &texture2D))) {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
            srvDesc.Format                    = desc.Format;
            srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels       = desc.MipLevels;
            srvDesc.Texture2D.MostDetailedMip = 0;

            if (SUCCEEDED(device->CreateShaderResourceView(texture2D.Get(), &srvDesc, &texture.mShaderResourceView))) {
                mTextures[name] = texture;
            }

            return true;
        }

        return false;
    }

    bool TextureManager::LoadFromDDSFile(const Path& ddsFile, const str& name) {
        TexMetadata metadata;
        ScratchImage scratchImage;
        HRESULT hr =
          DirectX::LoadFromDDSFile(AnsiToWide(ddsFile.Str()).c_str(), DDS_FLAGS_NONE, &metadata, scratchImage);
        if (FAILED(hr)) { return false; }

        TextureData texData;
        texData.mWidth  = metadata.width;
        texData.mHeight = metadata.height;

        hr = CreateShaderResourceView(mContext.GetDevice(),
                                      scratchImage.GetImages(),
                                      scratchImage.GetImageCount(),
                                      metadata,
                                      &texData.mShaderResourceView);
        if (FAILED(hr)) { return false; }

        mTextures[name] = texData;
        return true;
    }

    std::optional<TextureData> TextureManager::GetTexture(const str& name) {
        if (name.empty()) { return std::nullopt; }
        if (!mTextures.contains(name)) { return std::nullopt; }
        return mTextures[name];
    }
}  // namespace x