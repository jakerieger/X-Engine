#include "BloomEffect.hpp"

namespace x {
    bool BloomEffect::Initialize() {
        _computeShader.LoadFromFile(R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\Bloom.hlsl)");
        return CreateResources();
    }

    bool BloomEffect::CreateResources() {
        return CreateConstantBuffer<BloomParams>();
    }

    void BloomEffect::UpdateConstants() {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(_renderer.GetContext()->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* params = CAST<BloomParams*>(mapped.pData);

            params->threshold    = _threshold;
            params->intensity    = _intensity;
            params->screenWidth  = CAST<f32>(_width);
            params->screenHeight = CAST<f32>(_height);

            _renderer.GetContext()->Unmap(_constantBuffer.Get(), 0);
        }
    }
}