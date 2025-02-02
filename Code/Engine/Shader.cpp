#include "Shader.hpp"
#include "Common/Str.hpp"
#include "Renderer.hpp"


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
        if (FAILED(hr)) {
            if (errorBlob) {
                const char* msg = CAST<const char*>(errorBlob->GetBufferPointer());
                size_t msgSize  = errorBlob->GetBufferSize();
                PANIC("Shader file failed to compile: %s\n - Error: %s", filename.c_str(), msg);
                errorBlob->Release();
            }
        }

        hr = D3DReflect(_shaderBlob->GetBufferPointer(),
                        _shaderBlob->GetBufferSize(),
                        IID_ID3D11ShaderReflection,
                        &_reflection);
        PANIC_IF_FAILED(hr, "Failed to capture shader reflection data.")
    }

    void IShader::InitFromMemory(const u8* data, const size_t size) {
        auto hr = D3DCreateBlob(size, &_shaderBlob);
        PANIC_IF_FAILED(hr, "Failed to create blob.")

        memcpy(_shaderBlob->GetBufferPointer(), data, size);

        hr = D3DReflect(_shaderBlob->GetBufferPointer(),
                        _shaderBlob->GetBufferSize(),
                        IID_ID3D11ShaderReflection,
                        &_reflection);
        PANIC_IF_FAILED(hr, "Failed to capture shader reflection data.")
    }

    D3D11_SHADER_DESC IShader::GetShaderDesc() const {
        D3D11_SHADER_DESC desc{};
        PANIC_IF_FAILED(_reflection->GetDesc(&desc), "Failed to get reflection description from shader.")
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
        PANIC_IF_FAILED(hr, "")
        CreateInputLayout();
    }

    void VertexShader::LoadFromMemory(const u8* data, const size_t size) {
        InitFromMemory(data, size);
        const auto hr = _renderer.GetDevice()->CreateVertexShader(_shaderBlob->GetBufferPointer(),
                                                                  _shaderBlob->GetBufferSize(),
                                                                  None,
                                                                  RCAST<ID3D11VertexShader**>(_shader.GetAddressOf()));
        PANIC_IF_FAILED(hr, "")
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
            PANIC_IF_FAILED(hr, "")

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
        PANIC_IF_FAILED(hr, "")
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
        const auto hr = _renderer.GetDevice()->CreatePixelShader(_shaderBlob->GetBufferPointer(),
                                                                 _shaderBlob->GetBufferSize(),
                                                                 None,
                                                                 RCAST<ID3D11PixelShader**>(_shader.GetAddressOf()));
        PANIC_IF_FAILED(hr, "")
    }

    void PixelShader::LoadFromMemory(const u8* data, const size_t size) {
        InitFromMemory(data, size);
        const auto hr = _renderer.GetDevice()->CreatePixelShader(_shaderBlob->GetBufferPointer(),
                                                                 _shaderBlob->GetBufferSize(),
                                                                 None,
                                                                 RCAST<ID3D11PixelShader**>(_shader.GetAddressOf()));
        PANIC_IF_FAILED(hr, "")
    }

    void PixelShader::Bind() {
        const auto pixelShader = CAST<ID3D11PixelShader*>(_shader.Get());
        if (!pixelShader) {
            printf("Pixel shader is null.\n");
            return;
        }
        _renderer.GetContext()->PSSetShader(pixelShader, None, 0);
    }
    #pragma endregion

    #pragma region ComputeShader
    void ComputeShader::LoadFromFile(const str& filename, const char* entryPoint) {
        InitFromFile(filename, entryPoint, kComputeShaderModel);
        const auto hr =
            _renderer.GetDevice()->CreateComputeShader(_shaderBlob->GetBufferPointer(),
                                                       _shaderBlob->GetBufferSize(),
                                                       None,
                                                       RCAST<ID3D11ComputeShader**>(_shader.GetAddressOf()));
        PANIC_IF_FAILED(hr, "")
        ExtractThreadGroupSize();
    }

    void ComputeShader::LoadFromMemory(const u8* data, const size_t size) {
        InitFromMemory(data, size);
        const auto hr =
            _renderer.GetDevice()->CreateComputeShader(_shaderBlob->GetBufferPointer(),
                                                       _shaderBlob->GetBufferSize(),
                                                       None,
                                                       RCAST<ID3D11ComputeShader**>(_shader.GetAddressOf()));
        PANIC_IF_FAILED(hr, "")
        ExtractThreadGroupSize();
    }

    void ComputeShader::Bind() const {
        _renderer.GetContext()->CSSetShader(CAST<ID3D11ComputeShader*>(_shader.Get()), None, 0);
    }

    void ComputeShader::Dispatch(const u32 groupSizeX, const u32 groupSizeY, const u32 groupSizeZ) const {
        _renderer.GetContext()->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
    }

    void ComputeShader::DispatchWithThreadCount(const u32 threadCountX,
                                                const u32 threadCountY,
                                                const u32 threadCountZ) const {
        const u32 groupSizeX = (threadCountX + _threadGroupSizeX - 1) / _threadGroupSizeX;
        const u32 groupSizeY = (threadCountY + _threadGroupSizeY - 1) / _threadGroupSizeY;
        const u32 groupSizeZ = (threadCountZ + _threadGroupSizeZ - 1) / _threadGroupSizeZ;
        Dispatch(groupSizeX, groupSizeY, groupSizeZ);
    }

    void ComputeShader::ExtractThreadGroupSize() {
        const u32 result = _reflection->GetThreadGroupSize(&_threadGroupSizeX, &_threadGroupSizeY, &_threadGroupSizeZ);
        if (!result) {
            PANIC("Failed to get thread group size from reflection data.");
        }
        if (_threadGroupSizeX == 0 || _threadGroupSizeY == 0 || _threadGroupSizeZ == 0) {
            PANIC("Invalid thread group size in compute shader");
        }
    }
    #pragma endregion
}