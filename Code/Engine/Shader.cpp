#include "Shader.hpp"
#include "Common/Str.hpp"
#include "RenderContext.hpp"

#ifdef NDEBUG
static constexpr auto kCompileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#else
static constexpr auto kCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

static auto kVertexShaderModel  = "vs_5_0";
static auto kPixelShaderModel   = "ps_5_0";
static auto kComputeShaderModel = "cs_5_0";

namespace x {
#pragma region IShader
    void IShader::InitFromFile(const str& filename, const char* entryPoint, const char* target) {
        ComPtr<ID3DBlob> errorBlob;
        auto hr = D3DCompileFromFile(AnsiToWide(filename).c_str(),
                                     nullptr,
                                     D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                     entryPoint,
                                     target,
                                     kCompileFlags,
                                     0,
                                     &mShaderBlob,
                                     &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) {
                const char* msg = CAST<const char*>(errorBlob->GetBufferPointer());
                size_t msgSize  = errorBlob->GetBufferSize();
                X_PANIC("Shader file failed to compile: %s\n - Error: %s", filename.c_str(), msg);
                errorBlob->Release();
            }
        }

        hr = D3DReflect(mShaderBlob->GetBufferPointer(),
                        mShaderBlob->GetBufferSize(),
                        IID_ID3D11ShaderReflection,
                        &mReflection);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to capture shader reflection data.")
    }

    void IShader::InitFromMemory(const u8* data, const size_t size) {
        auto hr = D3DCreateBlob(size, &mShaderBlob);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create blob.")

        memcpy(mShaderBlob->GetBufferPointer(), data, size);

        hr = D3DReflect(mShaderBlob->GetBufferPointer(),
                        mShaderBlob->GetBufferSize(),
                        IID_ID3D11ShaderReflection,
                        &mReflection);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to capture shader reflection data.")
    }

    D3D11_SHADER_DESC IShader::GetShaderDesc() const {
        D3D11_SHADER_DESC desc {};
        const auto hr = mReflection->GetDesc(&desc);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to get reflection description from shader.")
        return desc;
    }
#pragma endregion

#pragma region VertexShader
    void VertexShader::LoadFromFile(const str& filename, const char* entryPoint) {
        InitFromFile(filename, entryPoint, kVertexShaderModel);
        const auto hr = mRenderer.GetDevice()->CreateVertexShader(mShaderBlob->GetBufferPointer(),
                                                                  mShaderBlob->GetBufferSize(),
                                                                  nullptr,
                                                                  RCAST<ID3D11VertexShader**>(mShader.GetAddressOf()));
        X_PANIC_ASSERT(SUCCEEDED(hr), "")
        CreateInputLayout();
    }

    void VertexShader::LoadFromMemory(const u8* data, const size_t size) {
        InitFromMemory(data, size);
        const auto hr = mRenderer.GetDevice()->CreateVertexShader(mShaderBlob->GetBufferPointer(),
                                                                  mShaderBlob->GetBufferSize(),
                                                                  nullptr,
                                                                  RCAST<ID3D11VertexShader**>(mShader.GetAddressOf()));
        X_PANIC_ASSERT(SUCCEEDED(hr), "")
        CreateInputLayout();
    }

    void VertexShader::Bind() const {
        const auto vs = CAST<ID3D11VertexShader*>(mShader.Get());
        if (!vs) {
            printf("Vertex shader is null.\n");
            return;
        }
        auto* context = mRenderer.GetDeviceContext();
        context->VSSetShader(vs, nullptr, 0);
        context->IASetInputLayout(mInputLayout.Get());
    }

    void VertexShader::CreateInputLayout() {
        const auto desc = GetShaderDesc();
        vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
        for (u32 i = 0; i < desc.InputParameters; i++) {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            const auto hr = mReflection->GetInputParameterDesc(i, &paramDesc);
            X_PANIC_ASSERT(SUCCEEDED(hr), "")

            D3D11_INPUT_ELEMENT_DESC elemDesc;
            elemDesc.SemanticName         = paramDesc.SemanticName;
            elemDesc.SemanticIndex        = paramDesc.SemanticIndex;
            elemDesc.InputSlot            = 0;
            elemDesc.AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
            elemDesc.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
            elemDesc.InstanceDataStepRate = 0;
            elemDesc.Format               = GetDXGIFormat(paramDesc.Mask, paramDesc.ComponentType);

            inputLayoutDesc.push_back(elemDesc);
        }

        const auto hr = mRenderer.GetDevice()->CreateInputLayout(inputLayoutDesc.data(),
                                                                 CAST<u32>(inputLayoutDesc.size()),
                                                                 mShaderBlob->GetBufferPointer(),
                                                                 mShaderBlob->GetBufferSize(),
                                                                 &mInputLayout);
        X_PANIC_ASSERT(SUCCEEDED(hr), "")
    }

    DXGI_FORMAT VertexShader::GetDXGIFormat(const u8 mask, const D3D_REGISTER_COMPONENT_TYPE componentType) {
        u32 numComponents = 0;
        for (u32 i = 0; i < 4; i++) {
            if (mask & (1 << i)) { numComponents++; }
        }

        switch (componentType) {
            case D3D_REGISTER_COMPONENT_FLOAT32:
                switch (numComponents) {
                    case 1:
                        return DXGI_FORMAT_R32_FLOAT;
                    case 2:
                        return DXGI_FORMAT_R32G32_FLOAT;
                    case 3:
                        return DXGI_FORMAT_R32G32B32_FLOAT;
                    default:
                        return DXGI_FORMAT_R32G32B32A32_FLOAT;
                }
            case D3D_REGISTER_COMPONENT_UNKNOWN:
                printf("WARNING: Unknown component type encountered in shader input.\n");
                return DXGI_FORMAT_UNKNOWN;
            case D3D_REGISTER_COMPONENT_UINT32:
                switch (numComponents) {
                    case 1:
                        return DXGI_FORMAT_R32_UINT;
                    case 2:
                        return DXGI_FORMAT_R32G32_UINT;
                    case 3:
                        return DXGI_FORMAT_R32G32B32_UINT;
                    default:
                        return DXGI_FORMAT_R32G32B32A32_UINT;
                }
            case D3D_REGISTER_COMPONENT_SINT32:
                switch (numComponents) {
                    case 1:
                        return DXGI_FORMAT_R32_SINT;
                    case 2:
                        return DXGI_FORMAT_R32G32_SINT;
                    case 3:
                        return DXGI_FORMAT_R32G32B32_SINT;
                    default:
                        return DXGI_FORMAT_R32G32B32A32_SINT;
                }
        }
        return DXGI_FORMAT_UNKNOWN;
    }
#pragma endregion

#pragma region PixelShader
    void PixelShader::LoadFromFile(const str& filename, const char* entryPoint) {
        InitFromFile(filename, entryPoint, kPixelShaderModel);
        const auto hr = mRenderer.GetDevice()->CreatePixelShader(mShaderBlob->GetBufferPointer(),
                                                                 mShaderBlob->GetBufferSize(),
                                                                 nullptr,
                                                                 RCAST<ID3D11PixelShader**>(mShader.GetAddressOf()));
        X_PANIC_ASSERT(SUCCEEDED(hr), "")
    }

    void PixelShader::LoadFromMemory(const u8* data, const size_t size) {
        InitFromMemory(data, size);
        const auto hr = mRenderer.GetDevice()->CreatePixelShader(mShaderBlob->GetBufferPointer(),
                                                                 mShaderBlob->GetBufferSize(),
                                                                 nullptr,
                                                                 RCAST<ID3D11PixelShader**>(mShader.GetAddressOf()));
        X_PANIC_ASSERT(SUCCEEDED(hr), "")
    }

    void PixelShader::Bind() {
        const auto pixelShader = CAST<ID3D11PixelShader*>(mShader.Get());
        if (!pixelShader) {
            printf("Pixel shader is null.\n");
            return;
        }
        mRenderer.GetDeviceContext()->PSSetShader(pixelShader, nullptr, 0);
    }
#pragma endregion

#pragma region ComputeShader
    void ComputeShader::LoadFromFile(const str& filename, const char* entryPoint) {
        InitFromFile(filename, entryPoint, kComputeShaderModel);
        const auto hr =
          mRenderer.GetDevice()->CreateComputeShader(mShaderBlob->GetBufferPointer(),
                                                     mShaderBlob->GetBufferSize(),
                                                     nullptr,
                                                     RCAST<ID3D11ComputeShader**>(mShader.GetAddressOf()));
        X_PANIC_ASSERT(SUCCEEDED(hr), "")
        ExtractThreadGroupSize();
    }

    void ComputeShader::LoadFromMemory(const u8* data, const size_t size) {
        InitFromMemory(data, size);
        const auto hr =
          mRenderer.GetDevice()->CreateComputeShader(mShaderBlob->GetBufferPointer(),
                                                     mShaderBlob->GetBufferSize(),
                                                     nullptr,
                                                     RCAST<ID3D11ComputeShader**>(mShader.GetAddressOf()));
        X_PANIC_ASSERT(SUCCEEDED(hr), "")
        ExtractThreadGroupSize();
    }

    void ComputeShader::Bind() const {
        mRenderer.GetDeviceContext()->CSSetShader(CAST<ID3D11ComputeShader*>(mShader.Get()), nullptr, 0);
    }

    void ComputeShader::Dispatch(const u32 groupSizeX, const u32 groupSizeY, const u32 groupSizeZ) const {
        mRenderer.GetDeviceContext()->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
    }

    void ComputeShader::DispatchWithThreadCount(const u32 threadCountX,
                                                const u32 threadCountY,
                                                const u32 threadCountZ) const {
        const u32 groupSizeX = (threadCountX + mThreadGroupSizeX - 1) / mThreadGroupSizeX;
        const u32 groupSizeY = (threadCountY + mThreadGroupSizeY - 1) / mThreadGroupSizeY;
        const u32 groupSizeZ = (threadCountZ + mThreadGroupSizeZ - 1) / mThreadGroupSizeZ;
        Dispatch(groupSizeX, groupSizeY, groupSizeZ);
    }

    void ComputeShader::ExtractThreadGroupSize() {
        const u32 result = mReflection->GetThreadGroupSize(&mThreadGroupSizeX, &mThreadGroupSizeY, &mThreadGroupSizeZ);
        if (!result) { X_PANIC("Failed to get thread group size from reflection data."); }
        if (mThreadGroupSizeX == 0 || mThreadGroupSizeY == 0 || mThreadGroupSizeZ == 0) {
            X_PANIC("Invalid thread group size in compute shader");
        }
    }

    void GraphicsShader::Bind() const {
        mVertexShader->Bind();
        mPixelShader->Bind();
    }
#pragma endregion
}  // namespace x