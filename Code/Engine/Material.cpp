#include "Material.hpp"
#include "RenderContext.hpp"

namespace x {
    PBRMaterial::PBRMaterial(RenderContext& renderer) : _renderer(renderer) {
        _vertexShader = make_unique<VertexShader>(renderer);
        _pixelShader  = make_unique<PixelShader>(renderer);

        const auto pbrShader = R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\PBR_TexMaps.hlsl)";
        _vertexShader->LoadFromFile(pbrShader);
        _pixelShader->LoadFromFile(pbrShader);
        CreateBuffers();
    }

    shared_ptr<PBRMaterial> PBRMaterial::Create(RenderContext& renderer) {
        return make_shared<PBRMaterial>(renderer);
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
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create transforms constant buffer.")

        D3D11_BUFFER_DESC lightsBufDesc;
        lightsBufDesc.ByteWidth           = sizeof(LightState);
        lightsBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        lightsBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        lightsBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        lightsBufDesc.MiscFlags           = 0;
        lightsBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&lightsBufDesc, None, &_lightsCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create lights constant buffer.")

        D3D11_BUFFER_DESC materialBufDesc;
        materialBufDesc.ByteWidth           = sizeof(MaterialProperties);
        materialBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        materialBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        materialBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        materialBufDesc.MiscFlags           = 0;
        materialBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&materialBufDesc, None, &_materialCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create material constant buffer.")

        D3D11_BUFFER_DESC cameraBufDesc;
        cameraBufDesc.ByteWidth           = sizeof(Float4);
        cameraBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        cameraBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        cameraBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        cameraBufDesc.MiscFlags           = 0;
        cameraBufDesc.StructureByteStride = 0;

        hr = _renderer.GetDevice()->CreateBuffer(&cameraBufDesc, None, &_cameraCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create camera constant buffer.")
    }

    void PBRMaterial::UpdateBuffers(const TransformMatrices& transforms,
                                    const LightState& lights,
                                    const MaterialProperties& matProps,
                                    const Float3& eyePosition) {
        auto* context = _renderer.GetDeviceContext();

        // Update transform buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        auto hr = context->Map(_transformsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map transforms buffer.")
        memcpy(mapped.pData, &transforms, sizeof(transforms));
        context->Unmap(_transformsCB.Get(), 0);

        // Update light state buffer
        hr = context->Map(_lightsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map lights buffer.")
        memcpy(mapped.pData, &lights, sizeof(lights));
        context->Unmap(_lightsCB.Get(), 0);

        // Update material properties buffer
        hr = context->Map(_materialCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map material buffer.")
        memcpy(mapped.pData, &matProps, sizeof(MaterialProperties));
        context->Unmap(_materialCB.Get(), 0);

        // Camera buffer
        hr = context->Map(_cameraCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map camera buffer.")
        Float4 paddedPos = Float4(eyePosition.x, eyePosition.y, eyePosition.z, 0.0f);
        memcpy(mapped.pData, &paddedPos, sizeof(paddedPos));
        context->Unmap(_cameraCB.Get(), 0);
    }

    void PBRMaterial::BindShaders() {
        _vertexShader->Bind();
        _pixelShader->Bind();
    }

    void PBRMaterial::BindBuffers() {
        auto* context = _renderer.GetDeviceContext();
        context->VSSetConstantBuffers(0, 1, _transformsCB.GetAddressOf());
        context->PSSetConstantBuffers(1, 1, _lightsCB.GetAddressOf());
        context->VSSetConstantBuffers(1, 1, _lightsCB.GetAddressOf());
        context->PSSetConstantBuffers(2, 1, _materialCB.GetAddressOf());
        context->PSSetConstantBuffers(3, 1, _cameraCB.GetAddressOf());
    }

    //=====================================================================================================================//
    //=====================================================================================================================//

    void PBRMaterialInstance::SetAlbedo(const Float3& albedo) {
        _albedo = albedo;
    }

    void PBRMaterialInstance::SetMetallic(f32 metallic) {
        _metallic = metallic;
    }

    void PBRMaterialInstance::SetRoughness(f32 roughness) {
        _roughness = roughness;
    }

    void PBRMaterialInstance::SetAO(f32 ao) {
        _ao = ao;
    }

    void PBRMaterialInstance::SetEmissive(const Float3& emissive, f32 strength) {
        _emissive         = emissive;
        _emissiveStrength = strength;
    }

    void PBRMaterialInstance::SetAlbedoMap(const ResourceHandle<Texture2D>& albedo) {
        _albedoMap = albedo;
    }

    void PBRMaterialInstance::SetMetallicMap(const ResourceHandle<Texture2D>& metallic) {
        _metallicMap = metallic;
    }

    void PBRMaterialInstance::SetRoughnessMap(const ResourceHandle<Texture2D>& roughness) {
        _roughnessMap = roughness;
    }

    void PBRMaterialInstance::SetNormalMap(const ResourceHandle<Texture2D>& normal) {
        _normalMap = normal;
    }

    void PBRMaterialInstance::SetTextureMaps(const ResourceHandle<Texture2D>& albedo,
                                             const ResourceHandle<Texture2D>& metallic,
                                             const ResourceHandle<Texture2D>& roughness,
                                             const ResourceHandle<Texture2D>& normal) {
        SetAlbedoMap(albedo);
        SetMetallicMap(metallic);
        SetRoughnessMap(roughness);
        SetNormalMap(normal);
    }

    void PBRMaterialInstance::Bind(const TransformMatrices& transforms,
                                   const LightState& lights,
                                   const Float3 eyePos) const {
        UpdateInstanceParams(transforms, lights, eyePos);

        _baseMaterial->BindShaders();

        // ReSharper disable CppCStyleCast
        if (_albedoMap.Get()) { _albedoMap->Bind((u32)TextureMapSlot::Albedo); }
        if (_metallicMap.Get()) { _metallicMap->Bind((u32)TextureMapSlot::Metallic); }
        if (_roughnessMap.Get()) { _roughnessMap->Bind((u32)TextureMapSlot::Roughness); }
        if (_normalMap.Get()) { _normalMap->Bind((u32)TextureMapSlot::Normal); }
        // ReSharper restore CppCStyleCast

        _baseMaterial->BindBuffers();
    }

    void PBRMaterialInstance::Unbind() const {
        // ReSharper disable CppCStyleCast
        if (_albedoMap.Get()) { _albedoMap->Unbind((u32)TextureMapSlot::Albedo); }
        if (_metallicMap.Get()) { _metallicMap->Unbind((u32)TextureMapSlot::Metallic); }
        if (_roughnessMap.Get()) { _roughnessMap->Unbind((u32)TextureMapSlot::Roughness); }
        if (_normalMap.Get()) { _normalMap->Unbind((u32)TextureMapSlot::Normal); }
        // ReSharper restore CppCStyleCast
    }

    void PBRMaterialInstance::UpdateInstanceParams(const TransformMatrices& transforms,
                                                   const LightState& lights,
                                                   const Float3 eyePos) const {
        PBRMaterial::MaterialProperties materialProperties;
        materialProperties.albedo           = _albedo;
        materialProperties.metallic         = _metallic;
        materialProperties.roughness        = _roughness;
        materialProperties.ao               = _ao;
        materialProperties.emissive         = _emissive;
        materialProperties.emissiveStrength = _emissiveStrength;

        _baseMaterial->UpdateBuffers(transforms, lights, materialProperties, eyePos);
    }
}