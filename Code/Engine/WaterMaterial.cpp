// Author: Jake Rieger
// Created: 3/7/2025.
//

#include "WaterMaterial.hpp"
#include "ShaderManager.hpp"

namespace x {
    WaterMaterial::WaterMaterial(RenderContext& context, bool transparent) : IMaterial(context, transparent) {
        mShader = ShaderManager::GetGraphicsShader(kWaterShaderId);
        CreateBuffers();
    }

    void WaterMaterial::Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const {
        BindShaders();
        BindBuffers();
        UpdateBuffers({transforms, lights, eyePos});
    }

    void WaterMaterial::Unbind() const {}

    void WaterMaterial::SetWaveTime(f32 waveTime) {
        mWaterParameters.mTime = waveTime;
    }

    void WaterMaterial::CreateBuffers() {
        mBuffers.Create(mContext, true);

        auto* device = mContext.GetDevice();
        D3D11_BUFFER_DESC desc;
        desc.ByteWidth           = sizeof(WaterParameters);
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;
        auto hr                  = device->CreateBuffer(&desc, nullptr, &mWaterParamsBuffer);
        if (FAILED(hr)) { throw std::runtime_error("Failed to create water parameter buffer"); }

        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth           = sizeof(WaveParameters);
        desc.Usage               = D3D11_USAGE_DYNAMIC;
        desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;
        hr                       = device->CreateBuffer(&desc, nullptr, &mWaveParamsBuffer);
        if (FAILED(hr)) { throw std::runtime_error("Failed to create wave parameter buffer"); }
    }

    void WaterMaterial::BindBuffers() const {
        mBuffers.Bind(mContext);
        const auto ctx = mContext.GetDeviceContext();
        ctx->VSSetConstantBuffers(kCustomBufferSlotIndex1, 1, mWaterParamsBuffer.GetAddressOf());
        ctx->PSSetConstantBuffers(kCustomBufferSlotIndex1, 1, mWaterParamsBuffer.GetAddressOf());
        ctx->VSSetConstantBuffers(kCustomBufferSlotIndex2, 1, mWaveParamsBuffer.GetAddressOf());
        ctx->PSSetConstantBuffers(kCustomBufferSlotIndex2, 1, mWaveParamsBuffer.GetAddressOf());
    }

    void WaterMaterial::UpdateBuffers(const MaterialParameters& params) const {
        mBuffers.Update(mContext, params);

        auto* ctx = mContext.GetDeviceContext();
        D3D11_MAPPED_SUBRESOURCE mapped;
        auto hr = ctx->Map(mWaterParamsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) { throw std::runtime_error("Failed to map Water parameter buffer"); }
        memcpy(mapped.pData, &mWaterParameters, sizeof(WaterParameters));
        ctx->Unmap(mWaterParamsBuffer.Get(), 0);

        ZeroMemory(&mapped, sizeof(mapped));
        hr = ctx->Map(mWaveParamsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr)) { throw std::runtime_error("Failed to map wave parameter buffer"); }
        memcpy(mapped.pData, &mWaveParameters, sizeof(WaveParameters));
        ctx->Unmap(mWaveParamsBuffer.Get(), 0);
    }
}  // namespace x