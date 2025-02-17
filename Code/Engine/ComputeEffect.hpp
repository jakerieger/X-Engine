#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "RenderContext.hpp"
#include "Shader.hpp"

namespace x {
    class IComputeEffect {
        friend class PostProcessSystem;

    public:
        explicit IComputeEffect(RenderContext& renderer) : _renderer(renderer), _computeShader(renderer) {}
        virtual ~IComputeEffect() = default;

        virtual bool Initialize() = 0;

        virtual void Execute(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output) {
            _computeShader.Bind();

            auto* context = _renderer.GetDeviceContext();
            context->CSSetShaderResources(0, 1, &input);
            context->CSSetUnorderedAccessViews(0, 1, &output, None);

            if (_constantBuffer) {
                UpdateConstants();
                context->CSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
            }

            _computeShader.DispatchWithThreadCount(_width, _height, 1);

            ID3D11ShaderResourceView* nullSRV  = None;
            ID3D11UnorderedAccessView* nullUAV = None;
            context->CSSetShaderResources(0, 1, &nullSRV);
            context->CSSetUnorderedAccessViews(0, 1, &nullUAV, None);
        }

        virtual void OnResize(const u32 width, const u32 height) {
            _width  = width;
            _height = height;
        }

        bool IsEnabled() const { return _enabled; }
        void SetEnabled(const bool enabled) { _enabled = enabled; }

        template<typename T>
        T* As() {
            return DCAST<T*>(this);
        }

    protected:
        RenderContext& _renderer;
        ComputeShader _computeShader;
        ComPtr<ID3D11Buffer> _constantBuffer;
        bool _enabled       = true;
        u32 _width          = 0;
        u32 _height         = 0;
        DXGI_FORMAT _format = DXGI_FORMAT_R8G8B8A8_UNORM;

        virtual bool CreateResources() { return true; }
        virtual void UpdateConstants() {}

        template<typename T>
        bool CreateConstantBuffer() {
            D3D11_BUFFER_DESC desc{};
            desc.ByteWidth      = sizeof(T);
            desc.Usage          = D3D11_USAGE_DYNAMIC;
            desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            return SUCCEEDED(_renderer.GetDevice()->CreateBuffer(&desc, None, &_constantBuffer));
        }
    };
}