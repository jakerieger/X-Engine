#include "Texture.hpp"
#include "RenderContext.hpp"
#include "Common/Str.hpp"

namespace x {
    void Texture2D::Bind(const u32 slot) const {
        mRenderer.GetDeviceContext()->PSSetShaderResources(slot, 1, mTextureView.GetAddressOf());
        mRenderer.GetDeviceContext()->PSSetSamplers(slot, 1, mSamplerState.GetAddressOf());
    }

    void Texture2D::Unbind(const u32 slot) const {
        ID3D11ShaderResourceView* nullSRV = None;
        ID3D11SamplerState* nullSS        = None;
        mRenderer.GetDeviceContext()->PSSetShaderResources(slot, 1, &nullSRV);
        mRenderer.GetDeviceContext()->PSSetSamplers(slot, 1, &nullSS);
    }
}  // namespace x