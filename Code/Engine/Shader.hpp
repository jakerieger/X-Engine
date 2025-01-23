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
        [[nodiscard]] D3D11_SHADER_DESC GetShaderDesc() const;
    };
    #pragma endregion

    #pragma region VertexShader
    class VertexShader final : public IShader {
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
    class PixelShader final : public IShader {
    public:
        explicit PixelShader(Renderer& renderer) : IShader(renderer) {}
        void LoadFromFile(const str& filename, const char* entryPoint = "PS_Main");
        void LoadFromMemory(const u8* data, size_t size);
        void Bind();
    };
    #pragma endregion

    #pragma region ComputeShader
    class ComputeShader final : public IShader {
        u32 _threadGroupSizeX = 0;
        u32 _threadGroupSizeY = 0;
        u32 _threadGroupSizeZ = 0;

    public:
        explicit ComputeShader(Renderer& renderer) : IShader(renderer) {}
        void LoadFromFile(const str& filename, const char* entryPoint = "CS_Main");
        void LoadFromMemory(const u8* data, size_t size);
        void Bind() const;
        void Dispatch(u32 groupSizeX, u32 groupSizeY, u32 groupSizeZ) const;
        void DispatchWithThreadCount(u32 threadCountX, u32 threadCountY, u32 threadCountZ) const;

        [[nodiscard]] u32 GetThreadGroupSizeX() const {
            return _threadGroupSizeX;
        }

        [[nodiscard]] u32 GetThreadGroupSizeY() const {
            return _threadGroupSizeY;
        }

        [[nodiscard]] u32 GetThreadGroupSizeZ() const {
            return _threadGroupSizeZ;
        }

    private:
        void ExtractThreadGroupSize();
    };
    #pragma endregion
}