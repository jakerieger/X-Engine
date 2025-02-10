#include "Texture.hpp"
#include "RenderContext.hpp"
#include "Common/Str.hpp"

namespace x {
    void Texture2D::Bind(const u32 slot) const {
        _renderer.GetDeviceContext()->PSSetShaderResources(slot, 1, _textureView.GetAddressOf());
        _renderer.GetDeviceContext()->PSSetSamplers(slot, 1, _samplerState.GetAddressOf());
    }

    void Texture2D::Unbind(const u32 slot) const {
        ID3D11ShaderResourceView* nullSRV = None;
        ID3D11SamplerState* nullSS        = None;
        _renderer.GetDeviceContext()->PSSetShaderResources(slot, 1, &nullSRV);
        _renderer.GetDeviceContext()->PSSetSamplers(slot, 1, &nullSS);
    }
}