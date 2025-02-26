#include "PostProcessSystem.hpp"
#include "RenderContext.hpp"
#include <algorithm>

#include "ScreenTexture_VS.h"
#include "ScreenTexture_PS.h"

namespace x {
    bool PostProcessSystem::Initialize(u32 width, u32 height) {
        mWidth  = width;
        mHeight = height;

        mFullscreenVS.LoadFromMemory(X_ARRAY_W_SIZE(kScreenTexture_VSBytes));
        mFullscreenPS.LoadFromMemory(X_ARRAY_W_SIZE(kScreenTexture_PSBytes));

        return CreateIntermediateTargets();
    }

    void PostProcessSystem::Resize(const u32 width, const u32 height) {
        mWidth  = width;
        mHeight = height;

        for (auto& target : mIntermediateTargets) {
            target.Reset();
        }

        CreateIntermediateTargets();

        for (const auto& effect : mEffects) {
            effect->OnResize(width, height);
        }
    }

    void PostProcessSystem::Execute(ID3D11ShaderResourceView* sceneInput, ID3D11RenderTargetView* finalOutput) {
        if (mEffects.empty()) {
            RenderToScreen(sceneInput, finalOutput);
            return;
        }

        // Count enabled effects
        const size_t enabledEffects =
          std::ranges::count_if(mEffects, [](const auto& effect) { return effect->IsEnabled(); });

        if (enabledEffects == 0) {
            RenderToScreen(sceneInput, finalOutput);
            return;
        }

        // Chain the compute effects together
        ID3D11ShaderResourceView* currentInput = sceneInput;
        size_t targetIndex                     = 0;

        // Process every enabled effect using compute shaders
        for (size_t i = 0; i < mEffects.size(); ++i) {
            if (!mEffects[i]->IsEnabled()) { continue; }

            // Select the appropriate output target
            bool isLastEffect =
              (i == mEffects.size() - 1) || std::none_of(std::next(mEffects.begin(), i + 1),
                                                         mEffects.end(),
                                                         [](const auto& effect) { return effect->IsEnabled(); });

            // For the last effect, we'll write to our final intermediate target
            ID3D11UnorderedAccessView* currentOutput = mIntermediateTargets[targetIndex].uav.Get();

            // Process this effect
            mEffects[i]->Execute(currentInput, currentOutput);

            // Set up for next iteration
            currentInput = mIntermediateTargets[targetIndex].srv.Get();
            targetIndex  = (targetIndex + 1) % mIntermediateTargets.size();
        }

        // After all effects have been processed, render the final result to screen
        RenderToScreen(currentInput, finalOutput);
    }

    bool PostProcessSystem::CreateIntermediateTargets() {
        if (mEffects.empty()) { return true; }

        mIntermediateTargets.resize(mEffects.size());

        for (size_t i = 0; i < mIntermediateTargets.size(); i++) {
            auto& target          = mIntermediateTargets[i];
            const auto dxgiFormat = mEffects[i]->mFormat;

            D3D11_TEXTURE2D_DESC desc {};
            desc.Width            = mWidth;
            desc.Height           = mHeight;
            desc.MipLevels        = 1;
            desc.ArraySize        = 1;
            desc.Format           = dxgiFormat;
            desc.SampleDesc.Count = 1;
            desc.Usage            = D3D11_USAGE_DEFAULT;
            desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

            auto* device = mRenderer.GetDevice();

            auto hr = device->CreateTexture2D(&desc, None, &target.texture);
            if (FAILED(hr)) { return false; }

            hr = device->CreateUnorderedAccessView(target.texture.Get(), None, &target.uav);
            if (FAILED(hr)) { return false; }

            hr = device->CreateShaderResourceView(target.texture.Get(), None, &target.srv);
            if (FAILED(hr)) { return false; }
        }

        return true;
    }

    void PostProcessSystem::RenderToScreen(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output) {
        auto* context = mRenderer.GetDeviceContext();
        context->OMSetRenderTargets(1, &output, None);

        mFullscreenVS.Bind();
        mFullscreenPS.Bind();

        context->PSSetShaderResources(0, 1, &input);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->Draw(3, 0);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        context->PSSetShaderResources(0, 1, &nullSRV);
    }
}  // namespace x