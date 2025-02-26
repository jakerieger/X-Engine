#include "Material.hpp"
#include "RenderContext.hpp"

#include "Lit_VS.h"
#include "Lit_PS.h"

namespace x {
    PBRMaterial::PBRMaterial(RenderContext& renderer) : mRenderer(renderer) {
        mVertexShader = make_unique<VertexShader>(renderer);
        mPixelShader  = make_unique<PixelShader>(renderer);

        mVertexShader->LoadFromMemory(X_ARRAY_W_SIZE(kLit_VSBytes));
        mPixelShader->LoadFromMemory(X_ARRAY_W_SIZE(kLit_PSBytes));

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

        auto hr = mRenderer.GetDevice()->CreateBuffer(&transformBufDesc, None, &mTransformsCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create transforms constant buffer.")

        D3D11_BUFFER_DESC lightsBufDesc;
        lightsBufDesc.ByteWidth           = sizeof(LightState);
        lightsBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        lightsBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        lightsBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        lightsBufDesc.MiscFlags           = 0;
        lightsBufDesc.StructureByteStride = 0;

        hr = mRenderer.GetDevice()->CreateBuffer(&lightsBufDesc, None, &mLightsCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create lights constant buffer.")

        D3D11_BUFFER_DESC materialBufDesc;
        materialBufDesc.ByteWidth           = sizeof(MaterialProperties);
        materialBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        materialBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        materialBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        materialBufDesc.MiscFlags           = 0;
        materialBufDesc.StructureByteStride = 0;

        hr = mRenderer.GetDevice()->CreateBuffer(&materialBufDesc, None, &mMaterialCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create material constant buffer.")

        D3D11_BUFFER_DESC cameraBufDesc;
        cameraBufDesc.ByteWidth           = sizeof(Float4);
        cameraBufDesc.Usage               = D3D11_USAGE_DYNAMIC;
        cameraBufDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
        cameraBufDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
        cameraBufDesc.MiscFlags           = 0;
        cameraBufDesc.StructureByteStride = 0;

        hr = mRenderer.GetDevice()->CreateBuffer(&cameraBufDesc, None, &mCameraCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create camera constant buffer.")
    }

    void PBRMaterial::UpdateBuffers(const TransformMatrices& transforms,
                                    const LightState& lights,
                                    const MaterialProperties& matProps,
                                    const Float3& eyePosition) {
        auto* context = mRenderer.GetDeviceContext();

        // Update transform buffer
        D3D11_MAPPED_SUBRESOURCE mapped;
        auto hr = context->Map(mTransformsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map transforms buffer.")
        memcpy(mapped.pData, &transforms, sizeof(transforms));
        context->Unmap(mTransformsCB.Get(), 0);

        // Update light state buffer
        hr = context->Map(mLightsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map lights buffer.")
        memcpy(mapped.pData, &lights, sizeof(lights));
        context->Unmap(mLightsCB.Get(), 0);

        // Update material properties buffer
        hr = context->Map(mMaterialCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map material buffer.")
        memcpy(mapped.pData, &matProps, sizeof(MaterialProperties));
        context->Unmap(mMaterialCB.Get(), 0);

        // Camera buffer
        hr = context->Map(mCameraCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to map camera buffer.")
        Float4 paddedPos = Float4(eyePosition.x, eyePosition.y, eyePosition.z, 0.0f);
        memcpy(mapped.pData, &paddedPos, sizeof(paddedPos));
        context->Unmap(mCameraCB.Get(), 0);
    }

    void PBRMaterial::BindShaders() {
        mVertexShader->Bind();
        mPixelShader->Bind();
    }

    void PBRMaterial::BindBuffers() {
        auto* context = mRenderer.GetDeviceContext();
        context->VSSetConstantBuffers(0, 1, mTransformsCB.GetAddressOf());
        context->PSSetConstantBuffers(1, 1, mLightsCB.GetAddressOf());
        context->VSSetConstantBuffers(1, 1, mLightsCB.GetAddressOf());
        context->PSSetConstantBuffers(2, 1, mMaterialCB.GetAddressOf());
        context->PSSetConstantBuffers(3, 1, mCameraCB.GetAddressOf());
    }

    //=====================================================================================================================//
    //=====================================================================================================================//

    void PBRMaterialInstance::SetAlbedo(const Float3& albedo) {
        mAlbedo = albedo;
    }

    void PBRMaterialInstance::SetMetallic(f32 metallic) {
        mMetallic = metallic;
    }

    void PBRMaterialInstance::SetRoughness(f32 roughness) {
        mRoughness = roughness;
    }

    void PBRMaterialInstance::SetAO(f32 ao) {
        mAo = ao;
    }

    void PBRMaterialInstance::SetEmissive(const Float3& emissive, f32 strength) {
        mEmissive         = emissive;
        mEmissiveStrength = strength;
    }

    void PBRMaterialInstance::SetAlbedoMap(const ResourceHandle<Texture2D>& albedo) {
        mAlbedoMap = albedo;
    }

    void PBRMaterialInstance::SetMetallicMap(const ResourceHandle<Texture2D>& metallic) {
        mMetallicMap = metallic;
    }

    void PBRMaterialInstance::SetRoughnessMap(const ResourceHandle<Texture2D>& roughness) {
        mRoughnessMap = roughness;
    }

    void PBRMaterialInstance::SetNormalMap(const ResourceHandle<Texture2D>& normal) {
        mNormalMap = normal;
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

        mBaseMaterial->BindShaders();

        // ReSharper disable CppCStyleCast
        if (mAlbedoMap.Get()) { mAlbedoMap->Bind((u32)TextureMapSlot::Albedo); }
        if (mMetallicMap.Get()) { mMetallicMap->Bind((u32)TextureMapSlot::Metallic); }
        if (mRoughnessMap.Get()) { mRoughnessMap->Bind((u32)TextureMapSlot::Roughness); }
        if (mNormalMap.Get()) { mNormalMap->Bind((u32)TextureMapSlot::Normal); }
        // ReSharper restore CppCStyleCast

        mBaseMaterial->BindBuffers();
    }

    void PBRMaterialInstance::Unbind() const {
        // ReSharper disable CppCStyleCast
        if (mAlbedoMap.Get()) { mAlbedoMap->Unbind((u32)TextureMapSlot::Albedo); }
        if (mMetallicMap.Get()) { mMetallicMap->Unbind((u32)TextureMapSlot::Metallic); }
        if (mRoughnessMap.Get()) { mRoughnessMap->Unbind((u32)TextureMapSlot::Roughness); }
        if (mNormalMap.Get()) { mNormalMap->Unbind((u32)TextureMapSlot::Normal); }
        // ReSharper restore CppCStyleCast
    }

    void PBRMaterialInstance::UpdateInstanceParams(const TransformMatrices& transforms,
                                                   const LightState& lights,
                                                   const Float3 eyePos) const {
        PBRMaterial::MaterialProperties materialProperties;
        materialProperties.albedo           = mAlbedo;
        materialProperties.metallic         = mMetallic;
        materialProperties.roughness        = mRoughness;
        materialProperties.ao               = mAo;
        materialProperties.emissive         = mEmissive;
        materialProperties.emissiveStrength = mEmissiveStrength;

        mBaseMaterial->UpdateBuffers(transforms, lights, materialProperties, eyePos);
    }
}  // namespace x