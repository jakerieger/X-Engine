#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Renderer.hpp"

namespace x {
    #pragma region IShader
    class IShader {
    protected:
        Renderer& _renderer;
        ComPtr<ID3DBlob> _shaderBlob;
        ComPtr<ID3D11ShaderReflection> _reflection;
        ComPtr<ID3D11DeviceChild> _shader;

    public:
        explicit IShader(Renderer& renderer) : _renderer(renderer) {}
        virtual ~IShader() = default;

    protected:
        void InitFromFile(const str& filename, const char* entryPoint, const char* target);
        void InitFromMemory(const u8* data, size_t size);
        D3D11_SHADER_DESC GetShaderDesc() const;
    };
    #pragma endregion

    #pragma region VertexShader
    class VertexShader : public IShader {
        ComPtr<ID3D11InputLayout> _inputLayout;

    public:
        explicit VertexShader(Renderer& renderer) : IShader(renderer) {}
        void LoadFromFile(const str& filename, const char* entryPoint = "VS_Main");
        void LoadFromMemory(const u8* data, size_t size);
        void Bind() const;

    private:
        void CreateInputLayout();
        DXGI_FORMAT GetDXGIFormat(u8 mask, D3D_REGISTER_COMPONENT_TYPE componentType);
    };
    #pragma endregion

    #pragma region PixelShader
    class PixelShader : public IShader {};
    #pragma endregion

    #pragma region ComputeShader
    class ComputeShader : public IShader {};
    #pragma endregion
}