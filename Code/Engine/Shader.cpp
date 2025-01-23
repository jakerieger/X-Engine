#include "Shader.hpp"
#include "Common/Str.hpp"

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
                                     None,
                                     D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                     entryPoint,
                                     target,
                                     kCompileFlags,
                                     0,
                                     &_shaderBlob,
                                     &errorBlob);
        DX_THROW_IF_FAILED(hr)
        hr = D3DReflect(_shaderBlob->GetBufferPointer(),
                        _shaderBlob->GetBufferSize(),
                        IID_ID3D11ShaderReflection,
                        &_reflection);
        DX_THROW_IF_FAILED(hr)
    }

    void IShader::InitFromMemory(const u8* data, const size_t size) {
        auto hr = D3DCreateBlob(size, &_shaderBlob);
        DX_THROW_IF_FAILED(hr)

        memcpy(_shaderBlob->GetBufferPointer(), data, size);

        hr = D3DReflect(_shaderBlob->GetBufferPointer(),
                        _shaderBlob->GetBufferSize(),
                        IID_ID3D11ShaderReflection,
                        &_reflection);
        DX_THROW_IF_FAILED(hr)
    }

    D3D11_SHADER_DESC IShader::GetShaderDesc() const {
        D3D11_SHADER_DESC desc{};
        DX_THROW_IF_FAILED(_reflection->GetDesc(&desc))
        return desc;
    }
    #pragma endregion

    #pragma region VertexShader
    void VertexShader::LoadFromFile(const str& filename, const char* entryPoint) {
        InitFromFile(filename, entryPoint, kVertexShaderModel);
        const auto hr = _renderer.GetDevice()->CreateVertexShader(_shaderBlob->GetBufferPointer(),
                                                                  _shaderBlob->GetBufferSize(),
                                                                  None,
                                                                  RCAST<ID3D11VertexShader**>(_shader.GetAddressOf()));
        DX_THROW_IF_FAILED(hr)
        CreateInputLayout();
    }

    void VertexShader::LoadFromMemory(const u8* data, const size_t size) {
        InitFromMemory(data, size);
        const auto hr = _renderer.GetDevice()->CreateVertexShader(_shaderBlob->GetBufferPointer(),
                                                                  _shaderBlob->GetBufferSize(),
                                                                  None,
                                                                  RCAST<ID3D11VertexShader**>(_shader.GetAddressOf()));
        DX_THROW_IF_FAILED(hr)
        CreateInputLayout();
    }

    void VertexShader::Bind() const {
        const auto vs = CAST<ID3D11VertexShader*>(_shader.Get());
        if (!vs) {
            printf("Vertex shader is null.\n");
            return;
        }
        auto* context = _renderer.GetContext();
        context->VSSetShader(vs, None, 0);
        context->IASetInputLayout(_inputLayout.Get());
    }

    void VertexShader::CreateInputLayout() {
        const auto desc = GetShaderDesc();
        vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
        for (u32 i = 0; i < desc.InputParameters; i++) {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            const auto hr = _reflection->GetInputParameterDesc(i, &paramDesc);
            DX_THROW_IF_FAILED(hr)

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

        const auto hr = _renderer.GetDevice()->CreateInputLayout(inputLayoutDesc.data(),
                                                                 CAST<u32>(inputLayoutDesc.size()),
                                                                 _shaderBlob->GetBufferPointer(),
                                                                 _shaderBlob->GetBufferSize(),
                                                                 &_inputLayout);
        DX_THROW_IF_FAILED(hr)
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
    #pragma endregion

    #pragma region ComputeShader
    #pragma endregion
}