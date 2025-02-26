// Author: Jake Rieger
// Created: 2/21/2025.
//

#include "TextureManager.hpp"
#include <ranges>

namespace x {
    namespace Editor {
        bool TextureManager::LoadFromMemory(const u8* data, int width, int height, int channels, const str& name) {
            TextureData texture {};
            texture.width  = width;
            texture.height = height;

            D3D11_TEXTURE2D_DESC desc {};
            desc.Width            = width;
            desc.Height           = height;
            desc.MipLevels        = 1;
            desc.ArraySize        = 1;
            desc.Format           = channels == 4 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage            = D3D11_USAGE_DEFAULT;
            desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA subResource = {};
            subResource.pSysMem                = data;
            subResource.SysMemPitch            = width * channels;

            ID3D11Texture2D* tex2D = None;
            auto* device           = mContext.GetDevice();
            if (SUCCEEDED(device->CreateTexture2D(&desc, &subResource, &tex2D))) {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format                          = desc.Format;
                srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels             = desc.MipLevels;
                srvDesc.Texture2D.MostDetailedMip       = 0;

                if (SUCCEEDED(device->CreateShaderResourceView(tex2D, &srvDesc, &texture.srv))) {
                    mTextures[name] = texture;
                }

                tex2D->Release();

                return true;
            }

            return false;
        }

        std::optional<TextureData> TextureManager::GetTexture(const str& name) {
            if (name.empty()) return std::nullopt;
            if (!mTextures.contains(name)) return std::nullopt;
            return mTextures[name];
        }
    }  // namespace Editor
}  // namespace x