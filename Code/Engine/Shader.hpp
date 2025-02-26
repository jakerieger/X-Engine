#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"

namespace x {
    class RenderContext;

#pragma region IShader
    class IShader {
    protected:
        RenderContext& mRenderer;
        ComPtr<ID3DBlob> mShaderBlob;
        ComPtr<ID3D11ShaderReflection> mReflection;
        ComPtr<ID3D11DeviceChild> mShader;

    public:
        explicit IShader(RenderContext& renderer) : mRenderer(renderer) {}
        virtual ~IShader() = default;

    protected:
        void InitFromFile(const str& filename, const char* entryPoint, const char* target);
        void InitFromMemory(const u8* data, size_t size);
        [[nodiscard]] D3D11_SHADER_DESC GetShaderDesc() const;
    };
#pragma endregion

#pragma region VertexShader
    class VertexShader final : public IShader {
        ComPtr<ID3D11InputLayout> mInputLayout;

    public:
        explicit VertexShader(RenderContext& renderer) : IShader(renderer) {}
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
        explicit PixelShader(RenderContext& renderer) : IShader(renderer) {}
        void LoadFromFile(const str& filename, const char* entryPoint = "PS_Main");
        void LoadFromMemory(const u8* data, size_t size);
        void Bind();
    };
#pragma endregion

#pragma region ComputeShader
    class ComputeShader final : public IShader {
        u32 mThreadGroupSizeX = 0;
        u32 mThreadGroupSizeY = 0;
        u32 mThreadGroupSizeZ = 0;

    public:
        explicit ComputeShader(RenderContext& renderer) : IShader(renderer) {}
        void LoadFromFile(const str& filename, const char* entryPoint = "CS_Main");
        void LoadFromMemory(const u8* data, size_t size);
        void Bind() const;
        void Dispatch(u32 groupSizeX, u32 groupSizeY, u32 groupSizeZ) const;
        void DispatchWithThreadCount(u32 threadCountX, u32 threadCountY, u32 threadCountZ) const;

        [[nodiscard]] u32 GetThreadGroupSizeX() const {
            return mThreadGroupSizeX;
        }

        [[nodiscard]] u32 GetThreadGroupSizeY() const {
            return mThreadGroupSizeY;
        }

        [[nodiscard]] u32 GetThreadGroupSizeZ() const {
            return mThreadGroupSizeZ;
        }

    private:
        void ExtractThreadGroupSize();
    };
#pragma endregion
}  // namespace x