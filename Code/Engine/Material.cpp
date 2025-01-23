#include "Material.hpp"

namespace x {
    PBRMaterial::PBRMaterial(Renderer& renderer) : _renderer(renderer) {
        _vertexShader = make_unique<VertexShader>(renderer);
        _pixelShader  = make_unique<PixelShader>(renderer);

        const auto pbrShader = R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\Unlit.hlsl)";
        _vertexShader->LoadFromFile(pbrShader);
        _pixelShader->LoadFromFile(pbrShader);
        CreateBuffers();
    }

    void PBRMaterial::Apply(const TransformMatrices& transformMatrices, const LightState& lightState) {
        UpdateBuffers(transformMatrices, lightState);

        _vertexShader->Bind();
        _pixelShader->Bind();

        auto* context = _renderer.GetContext();
        context->VSSetConstantBuffers(0, 1, _transformsCB.GetAddressOf());
        context->PSSetConstantBuffers(1, 1, _lightsCB.GetAddressOf());
        context->PSSetConstantBuffers(2, 1, _materialCB.GetAddressOf());
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
        DX_THROW_IF_FAILED(hr)

        D3D11_BUFFER_DESC lightsBufDesc;
        lightsBufDesc.ByteWidth           = sizeof(LightState);
        lightsBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        lightsBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        lightsBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        lightsBufDesc.MiscFlags           = 0;
        lightsBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&lightsBufDesc, None, &_lightsCB);
        DX_THROW_IF_FAILED(hr)

        D3D11_BUFFER_DESC materialBufDesc;
        materialBufDesc.ByteWidth           = sizeof(MaterialProperties);
        materialBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        materialBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        materialBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        materialBufDesc.MiscFlags           = 0;
        materialBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&materialBufDesc, None, &_materialCB);
        DX_THROW_IF_FAILED(hr)
    }

    void PBRMaterial::UpdateBuffers(const TransformMatrices& transformMatrices, const LightState& lightState) {
        auto* context = _renderer.GetContext();

        // Update transform buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        auto hr = context->Map(_transformsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        DX_THROW_IF_FAILED(hr)
        memcpy(mapped.pData, &transformMatrices, sizeof(transformMatrices));
        context->Unmap(_transformsCB.Get(), 0);

        // Update light state buffer
        hr = context->Map(_lightsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        DX_THROW_IF_FAILED(hr)
        memcpy(mapped.pData, &lightState, sizeof(lightState));
        context->Unmap(_lightsCB.Get(), 0);

        // Update material properties buffer
        hr = context->Map(_materialCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        DX_THROW_IF_FAILED(hr)
        memcpy(mapped.pData, &_materialProperties, sizeof(_materialProperties));
        context->Unmap(_materialCB.Get(), 0);
    }
}