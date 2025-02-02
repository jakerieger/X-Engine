#include "PostProcessSystem.hpp"
#include "Renderer.hpp"

#include <algorithm>

namespace x {
    bool PostProcessSystem::Initialize(u32 width, u32 height) {
        _width  = width;
        _height = height;

        const auto screenTextureShader = R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\ScreenTexture.hlsl)";
        _fullscreenVS.LoadFromFile(screenTextureShader);
        _fullscreenPS.LoadFromFile(screenTextureShader);

        return CreateIntermediateTargets();
    }

    void PostProcessSystem::OnResize(const u32 width, const u32 height) {
        _width  = width;
        _height = height;

        for (auto& target : _intermediateTargets) {
            target.Reset();
        }

        CreateIntermediateTargets();

        for (const auto& effect : _effects) {
            effect->OnResize(width, height);
        }
    }

    void PostProcessSystem::Execute(ID3D11ShaderResourceView* sceneInput, ID3D11RenderTargetView* finalOutput) {
        if (_effects.empty()) {
            RenderToScreen(sceneInput, finalOutput);
            return;
        }

        // Count enabled effects
        const size_t enabledEffects =
            std::ranges::count_if(_effects, [](const auto& effect) { return effect->IsEnabled(); });

        if (enabledEffects == 0) {
            RenderToScreen(sceneInput, finalOutput);
            return;
        }

        // Chain the compute effects together
        ID3D11ShaderResourceView* currentInput = sceneInput;
        size_t targetIndex                     = 0;

        // Process all effects using compute shaders
        for (size_t i = 0; i < _effects.size(); ++i) {
            if (!_effects[i]->IsEnabled())
                continue;

            const bool isLastEffect =
                (i == _effects.size() - 1) || std::none_of(std::next(_effects.begin(), i + 1),
                                                           _effects.end(),
                                                           [](const auto& effect) { return effect->IsEnabled(); });

            if (!isLastEffect) {
                // Use compute shader for intermediate effect
                _effects[i]->Execute(currentInput, _intermediateTargets[targetIndex].uav.Get());

                currentInput = _intermediateTargets[targetIndex].srv.Get();
                targetIndex  = (targetIndex + 1) % _intermediateTargets.size();
            } else {
                // For the last effect, render directly to screen using pixel shader
                RenderToScreen(currentInput, finalOutput);
                break;
            }
        }
    }

    bool PostProcessSystem::CreateIntermediateTargets() {
        _intermediateTargets.resize(_effects.size());

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

    void PostProcessSystem::RenderToScreen(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output) {
        auto* context = _renderer.GetContext();
        context->OMSetRenderTargets(1, &output, None);

        _fullscreenVS.Bind();
        _fullscreenPS.Bind();

        context->PSSetShaderResources(0, 1, &input);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->Draw(3, 0);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        context->PSSetShaderResources(0, 1, &nullSRV);
    }
}