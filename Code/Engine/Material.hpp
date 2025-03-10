#pragma once

#include "Lights.hpp"
#include "Common/Types.hpp"
#include "ResourceManager.hpp"
#include "Shader.hpp"
#include "TransformMatrices.hpp"

namespace x {
#pragma region Material Parameters
    struct MaterialParameters {
        TransformMatrices mTransformMatrices;
        std::optional<LightState> mLightState;
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

    struct MaterialBuffers {
        ComPtr<ID3D11Buffer> mTransforms;
        ComPtr<ID3D11Buffer> mLights;
        ComPtr<ID3D11Buffer> mCamera;
        bool mLit {true};  // Determines whether we create the Lights buffer

        // Set lit param to false if creating an unlit material
        void Create(const RenderContext& context, bool lit = true) {
            mLit         = lit;
            auto* device = context.GetDevice();

            D3D11_BUFFER_DESC transformBufDesc;
            transformBufDesc.ByteWidth           = sizeof(TransformMatrices);
            transformBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
            transformBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
            transformBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
            transformBufDesc.MiscFlags           = 0;
            transformBufDesc.StructureByteStride = 0;

            auto hr = device->CreateBuffer(&transformBufDesc, nullptr, &mTransforms);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create transforms constant buffer.")

            D3D11_BUFFER_DESC cameraBufDesc;
            cameraBufDesc.ByteWidth           = sizeof(Float4);
            cameraBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
            cameraBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
            cameraBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
            cameraBufDesc.MiscFlags           = 0;
            cameraBufDesc.StructureByteStride = 0;

            hr = device->CreateBuffer(&cameraBufDesc, nullptr, &mCamera);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create camera constant buffer.")

            if (lit) {
                D3D11_BUFFER_DESC lightsBufDesc;
                lightsBufDesc.ByteWidth           = sizeof(LightState);
                lightsBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
                lightsBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
                lightsBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
                lightsBufDesc.MiscFlags           = 0;
                lightsBufDesc.StructureByteStride = 0;

                hr = device->CreateBuffer(&lightsBufDesc, nullptr, &mLights);
                X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create lights constant buffer.")
            }
        }

        void Update(const RenderContext& context, const MaterialParameters& params) const {
            const auto ctx = context.GetDeviceContext();

            // Update transform buffer
            D3D11_MAPPED_SUBRESOURCE mapped;
            auto hr = ctx->Map(mTransforms.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map transforms buffer.")
            memcpy(mapped.pData, &params.mTransformMatrices, sizeof(params.mTransformMatrices));
            ctx->Unmap(mTransforms.Get(), 0);

            // Update camera state
            hr = ctx->Map(mCamera.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map camera buffer.")
            auto pos = Float4(params.mCameraEye.x, params.mCameraEye.y, params.mCameraEye.z, 0);
            memcpy(mapped.pData, &pos, sizeof(pos));
            ctx->Unmap(mCamera.Get(), 0);

            if (mLit) {
                if (!params.mLightState.has_value()) {
                    X_LOG_ERROR("Light state not set but params are defined as lit");
                    return;
                }

                const auto& lightState = *params.mLightState;

                // Update lights buffer
                hr = ctx->Map(mLights.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
                X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map lights buffer.")
                memcpy(mapped.pData, &lightState, sizeof(lightState));
                ctx->Unmap(mLights.Get(), 0);
            }
        }

        void Bind(const RenderContext& context) const {
            const auto ctx = context.GetDeviceContext();
            ctx->VSSetConstantBuffers(kTransformBufferSlotIndex, 1, mTransforms.GetAddressOf());
            ctx->PSSetConstantBuffers(kCameraBufferSlotIndex, 1, mCamera.GetAddressOf());
            if (mLit) {
                ctx->PSSetConstantBuffers(kLightBufferSlotIndex, 1, mLights.GetAddressOf());
                ctx->VSSetConstantBuffers(kLightBufferSlotIndex, 1, mLights.GetAddressOf());
            }
        }
    };
#pragma endregion

    struct MaterialUpdateParams {
        virtual ~MaterialUpdateParams() = default;
        X_CAST_DYNAMIC_AS
    };

    class IMaterial {
    public:
        explicit IMaterial(RenderContext& context, bool transparent = false)
            : mContext(context), mTransparent(transparent) {}
        virtual ~IMaterial() = default;

        X_CAST_DYNAMIC_AS

        virtual void Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const {}
        virtual void Bind(const TransformMatrices& transforms, const Float3 eyePos) const {}
        virtual void Bind() const {}
        virtual void Unbind() const {}

        bool Transparent() const {
            return mTransparent;
        }

    protected:
        RenderContext& mContext;
        shared_ptr<GraphicsShader> mShader;
        bool mTransparent;

        virtual void CreateBuffers()     = 0;
        virtual void BindBuffers() const = 0;

        virtual void UpdateBuffers(const MaterialParameters& params) const = 0;

        virtual void BindShaders() const {
            mShader->Bind();
        }
    };
}  // namespace x