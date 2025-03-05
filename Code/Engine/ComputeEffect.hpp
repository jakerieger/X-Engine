#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "RenderContext.hpp"
#include "Shader.hpp"

namespace x {
    class IComputeEffect {
        friend class PostProcessSystem;

    public:
        explicit IComputeEffect(RenderContext& renderer) : mRenderer(renderer), mComputeShader(renderer) {}
        virtual ~IComputeEffect() = default;

        virtual bool Initialize() = 0;

        virtual void Execute(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output) {
            mComputeShader.Bind();

            auto* context = mRenderer.GetDeviceContext();
            context->CSSetShaderResources(0, 1, &input);
            context->CSSetUnorderedAccessViews(0, 1, &output, nullptr);

            if (mConstantBuffer) {
                UpdateConstants();
                context->CSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
            }

            mComputeShader.DispatchWithThreadCount(mWidth, mHeight, 1);

            ID3D11ShaderResourceView* nullSRV  = nullptr;
            ID3D11UnorderedAccessView* nullUAV = nullptr;
            context->CSSetShaderResources(0, 1, &nullSRV);
            context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
        }

        virtual void OnResize(const u32 width, const u32 height) {
            mWidth  = width;
            mHeight = height;
        }

        bool IsEnabled() const {
            return mEnabled;
        }
        void SetEnabled(const bool enabled) {
            mEnabled = enabled;
        }

        template<typename T>
        T* As() {
            return DCAST<T*>(this);
        }

    protected:
        RenderContext& mRenderer;
        ComputeShader mComputeShader;
        ComPtr<ID3D11Buffer> mConstantBuffer;
        bool mEnabled       = true;
        u32 mWidth          = 0;
        u32 mHeight         = 0;
        DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

        virtual bool CreateResources() {
            return true;
        }
        virtual void UpdateConstants() {}

        template<typename T>
        bool CreateConstantBuffer() {
            D3D11_BUFFER_DESC desc {};
            desc.ByteWidth      = sizeof(T);
            desc.Usage          = D3D11_USAGE_DYNAMIC;
            desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            return SUCCEEDED(mRenderer.GetDevice()->CreateBuffer(&desc, nullptr, &mConstantBuffer));
        }
    };
}  // namespace x