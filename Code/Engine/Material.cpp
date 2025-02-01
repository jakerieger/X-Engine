#include "Material.hpp"

namespace x {
    PBRMaterial::PBRMaterial(Renderer& renderer) : _renderer(renderer) {
        _vertexShader = make_unique<VertexShader>(renderer);
        _pixelShader  = make_unique<PixelShader>(renderer);

        const auto pbrShader = R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\PBR_TexMaps.hlsl)";
        _vertexShader->LoadFromFile(pbrShader);
        _pixelShader->LoadFromFile(pbrShader);
        CreateBuffers();
    }

    void PBRMaterial::Apply(const TransformMatrices& transformMatrices,
                            const LightState& lightState,
                            const Float3& cameraPos) {
        UpdateBuffers(transformMatrices, lightState, cameraPos);

        _vertexShader->Bind();
        _pixelShader->Bind();

        auto* context = _renderer.GetContext();
        context->VSSetConstantBuffers(0, 1, _transformsCB.GetAddressOf());
        context->PSSetConstantBuffers(1, 1, _lightsCB.GetAddressOf());
        context->PSSetConstantBuffers(2, 1, _materialCB.GetAddressOf());
        context->PSSetConstantBuffers(3, 1, _cameraCB.GetAddressOf());
    }

    void PBRMaterial::SetAlbedo(const Float3& albedo) {
        _materialProperties.albedo = albedo;
    }

    void PBRMaterial::SetMetallic(f32 metallic) {
        _materialProperties.metallic = metallic;
    }

    void PBRMaterial::SetRoughness(f32 roughness) {
        _materialProperties.roughness = roughness;
    }

    void PBRMaterial::SetAO(f32 ao) {
        _materialProperties.ao = ao;
    }

    void PBRMaterial::SetEmissive(const Float3& emissive, f32 strength) {
        _materialProperties.emissive         = emissive;
        _materialProperties.emissiveStrength = strength;
    }

    void PBRMaterial::CreateBuffers() {
        D3D11_BUFFER_DESC transformBufDesc;
        transformBufDesc.ByteWidth           = sizeof(TransformMatrices);
        transformBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        transformBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        transformBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        transformBufDesc.MiscFlags           = 0;
        transformBufDesc.StructureByteStride = 0;

        auto hr = _renderer.GetDevice()->CreateBuffer(&transformBufDesc, None, &_transformsCB);
        PANIC_IF_FAILED(hr, "Failed to create transforms constant buffer.")

        D3D11_BUFFER_DESC lightsBufDesc;
        lightsBufDesc.ByteWidth           = sizeof(LightState);
        lightsBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        lightsBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        lightsBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        lightsBufDesc.MiscFlags           = 0;
        lightsBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&lightsBufDesc, None, &_lightsCB);
        PANIC_IF_FAILED(hr, "Failed to create lights constant buffer.")

        D3D11_BUFFER_DESC materialBufDesc;
        materialBufDesc.ByteWidth           = sizeof(MaterialProperties);
        materialBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        materialBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        materialBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        materialBufDesc.MiscFlags           = 0;
        materialBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&materialBufDesc, None, &_materialCB);
        PANIC_IF_FAILED(hr, "Failed to create material constant buffer.")

        D3D11_BUFFER_DESC cameraBufDesc;
        cameraBufDesc.ByteWidth           = sizeof(Float4);
        cameraBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        cameraBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        cameraBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        cameraBufDesc.MiscFlags           = 0;
        cameraBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&cameraBufDesc, None, &_cameraCB);
        PANIC_IF_FAILED(hr, "Failed to create camera constant buffer.")
    }

    void PBRMaterial::UpdateBuffers(const TransformMatrices& transformMatrices,
                                    const LightState& lightState,
                                    const Float3& cameraPos) {
        auto* context = _renderer.GetContext();

        // Update transform buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        auto hr = context->Map(_transformsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        PANIC_IF_FAILED(hr, "Failed to map transforms buffer.")
        memcpy(mapped.pData, &transformMatrices, sizeof(transformMatrices));
        context->Unmap(_transformsCB.Get(), 0);

        // Update light state buffer
        hr = context->Map(_lightsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        PANIC_IF_FAILED(hr, "Failed to map lights buffer.")
        memcpy(mapped.pData, &lightState, sizeof(lightState));
        context->Unmap(_lightsCB.Get(), 0);

        // Update material properties buffer
        hr = context->Map(_materialCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        PANIC_IF_FAILED(hr, "Failed to map material buffer.")
        memcpy(mapped.pData, &_materialProperties, sizeof(_materialProperties));
        context->Unmap(_materialCB.Get(), 0);

        // Camera buffer
        hr = context->Map(_cameraCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        PANIC_IF_FAILED(hr, "Failed to map camera buffer.")
        Float4 paddedPos = Float4(cameraPos.x, cameraPos.y, cameraPos.z, 0.0f);
        memcpy(mapped.pData, &paddedPos, sizeof(paddedPos));
        context->Unmap(_cameraCB.Get(), 0);
    }
}