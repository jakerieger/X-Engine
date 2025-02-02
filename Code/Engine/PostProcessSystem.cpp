#include "PostProcessSystem.hpp"
#include "ComputeEffect.hpp"
#include "Renderer.hpp"

namespace x {
    bool PostProcessSystem::Initialize(u32 width, u32 height) {
        _width  = width;
        _height = height;
        // return CreateIntermediateTargets();
        return true;
    }

    void PostProcessSystem::OnResize(const u32 width, const u32 height) {
        _width  = width;
        _height = height;

        // for (auto& target : _intermediateTargets) {
        //     target.Reset();
        // }
        //
        // CreateIntermediateTargets();
        //
        // for (const auto& effect : _effects) {
        //     effect->OnResize(width, height);
        // }
    }

    bool PostProcessSystem::CreateIntermediateTargets() {
        // _intermediateTargets.resize(_effects.size());

        for (auto& target : _intermediateTargets) {
            D3D11_TEXTURE2D_DESC desc{};
            desc.Width            = _width;
            desc.Height           = _height;
            desc.MipLevels        = 1;
            desc.ArraySize        = 1;
            desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage            = D3D11_USAGE_DEFAULT;
            desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

            auto* device = _renderer.GetDevice();

            auto hr = device->CreateTexture2D(&desc, None, &target.texture);
            if (FAILED(hr)) { return false; }

            hr = device->CreateUnorderedAccessView(target.texture.Get(), None, &target.uav);
            if (FAILED(hr)) { return false; }

            hr = device->CreateShaderResourceView(target.texture.Get(), None, &target.srv);
            if (FAILED(hr)) { return false; }
        }

        return true;
    }
}