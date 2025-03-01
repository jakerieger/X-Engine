#pragma once

#include "Lights.hpp"
#include "Common/Types.hpp"
#include "ResourceManager.hpp"
#include "Shader.hpp"
#include "TransformMatrices.hpp"

namespace x {
#pragma region Material Parameters
    struct LitMaterialParameters {
        TransformMatrices mTransformMatrices;
        LightState mLightState;
        Float3 mCameraEye;
    };

    struct UnlitMaterialParameters {
        TransformMatrices mTransformMatrices;
        Float3 mCameraEye;
    };
#pragma endregion

#pragma region Material Buffers
    static constexpr u32 kTransformBufferSlotIndex = 0;
    static constexpr u32 kLightBufferSlotIndex     = 1;
    static constexpr u32 kCameraBufferSlotIndex    = 2;
    // Additional slot indices for whatever additional constant buffers a material might define
    static constexpr u32 kCustomBufferSlotIndex1 = 3;
    static constexpr u32 kCustomBufferSlotIndex2 = 4;
    static constexpr u32 kCustomBufferSlotIndex3 = 5;

    struct LitMaterialBuffers {
        ComPtr<ID3D11Buffer> mTransforms;
        ComPtr<ID3D11Buffer> mLights;
        ComPtr<ID3D11Buffer> mCamera;

        void Create(const RenderContext& context) {
            auto* device = context.GetDevice();

            D3D11_BUFFER_DESC transformBufDesc;
            transformBufDesc.ByteWidth           = sizeof(TransformMatrices);
            transformBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
            transformBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
            transformBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
            transformBufDesc.MiscFlags           = 0;
            transformBufDesc.StructureByteStride = 0;

            auto hr = device->CreateBuffer(&transformBufDesc, None, &mTransforms);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create transforms constant buffer.")

            D3D11_BUFFER_DESC lightsBufDesc;
            lightsBufDesc.ByteWidth           = sizeof(LightState);
            lightsBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
            lightsBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
            lightsBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
            lightsBufDesc.MiscFlags           = 0;
            lightsBufDesc.StructureByteStride = 0;

            hr = device->CreateBuffer(&lightsBufDesc, None, &mLights);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create lights constant buffer.")

            D3D11_BUFFER_DESC cameraBufDesc;
            cameraBufDesc.ByteWidth           = sizeof(Float4);
            cameraBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
            cameraBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
            cameraBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
            cameraBufDesc.MiscFlags           = 0;
            cameraBufDesc.StructureByteStride = 0;

            hr = device->CreateBuffer(&cameraBufDesc, None, &mCamera);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create camera constant buffer.")
        }

        void Update(const RenderContext& context, const LitMaterialParameters& params) const {
            const auto ctx = context.GetDeviceContext();

            // Update transform buffer
            D3D11_MAPPED_SUBRESOURCE mapped;
            auto hr = ctx->Map(mTransforms.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map transforms buffer.")
            memcpy(mapped.pData, &params.mTransformMatrices, sizeof(params.mTransformMatrices));
            ctx->Unmap(mTransforms.Get(), 0);

            // Update lights buffer
            hr = ctx->Map(mLights.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map lights buffer.")
            memcpy(mapped.pData, &params.mLightState, sizeof(params.mLightState));
            ctx->Unmap(mLights.Get(), 0);

            // Update camera state
            hr = ctx->Map(mLights.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map camera buffer.")
            auto pos = Float4(params.mCameraEye.x, params.mCameraEye.y, params.mCameraEye.z, 0);
            memcpy(mapped.pData, &pos, sizeof(pos));
            ctx->Unmap(mLights.Get(), 0);
        }

        void Bind(const RenderContext& context) const {
            const auto ctx = context.GetDeviceContext();
            ctx->VSSetConstantBuffers(kTransformBufferSlotIndex, 1, mTransforms.GetAddressOf());
            ctx->PSSetConstantBuffers(kLightBufferSlotIndex, 1, mLights.GetAddressOf());
            ctx->VSSetConstantBuffers(kLightBufferSlotIndex, 1, mLights.GetAddressOf());
            ctx->PSSetConstantBuffers(kCameraBufferSlotIndex, 1, mCamera.GetAddressOf());
        }
    };

    struct UnlitMaterialBuffers {
        ComPtr<ID3D11Buffer> mTransforms;
        ComPtr<ID3D11Buffer> mCamera;

        void Create(RenderContext& context) {}

        void Update(const UnlitMaterialParameters& params) {}

        void Bind(RenderContext& context) {}
    };
#pragma endregion

    struct MaterialUpdateParams {
        virtual ~MaterialUpdateParams() = default;
        X_CAST_DYNAMIC_AS
    };

    class IMaterial {
    public:
        explicit IMaterial(RenderContext& context) : mContext(context) {}
        virtual ~IMaterial() = default;

        X_CAST_DYNAMIC_AS

        virtual void Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const {
            UpdateBuffers({transforms, lights, eyePos});
        }

        virtual void Bind(const TransformMatrices& transforms, const Float3 eyePos) const {
            UpdateBuffers({transforms, eyePos});
        }

        virtual void Bind() const {}
        virtual void Unbind() const {}

    protected:
        RenderContext& mContext;
        shared_ptr<GraphicsShader> mShader;

        virtual void CreateBuffers()     = 0;
        virtual void BindBuffers() const = 0;

        virtual void UpdateBuffers(const LitMaterialParameters& params) const {}

        virtual void UpdateBuffers(const UnlitMaterialParameters& params) const {}

        virtual void BindShaders() const {
            mShader->Bind();
        }
    };
}  // namespace x