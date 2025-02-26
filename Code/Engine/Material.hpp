#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Lights.hpp"
#include "ResourceManager.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "TransformMatrices.hpp"

namespace x {
    enum class TextureMapSlot : u32 {
        Albedo,
        Metallic,
        Roughness,
        AmbientOcclusion,
        Normal,
        ShadowZBuffer,
    };

    class PBRMaterial {
        RenderContext& mRenderer;
        ComPtr<ID3D11Buffer> mTransformsCB;
        ComPtr<ID3D11Buffer> mLightsCB;
        ComPtr<ID3D11Buffer> mMaterialCB;
        ComPtr<ID3D11Buffer> mCameraCB;
        unique_ptr<VertexShader> mVertexShader;
        unique_ptr<PixelShader> mPixelShader;

        struct alignas(16) MaterialProperties {
            Float3 albedo;
            f32 metallic;
            f32 roughness;
            f32 ao;
            f32 mPad[2];
            Float3 emissive;
            f32 emissiveStrength;
        };

    public:
        explicit PBRMaterial(RenderContext& renderer);
        static shared_ptr<PBRMaterial> Create(RenderContext& renderer);

    private:
        friend class PBRMaterialInstance;

        void CreateBuffers();
        void UpdateBuffers(const TransformMatrices& transforms,
                           const LightState& lights,
                           const MaterialProperties& matProps,
                           const Float3& eyePosition);
        void BindBuffers();
        void BindShaders();
    };

    class PBRMaterialInstance {
        // Base material handle
        shared_ptr<PBRMaterial> mBaseMaterial;

        // Texture maps for this instance
        ResourceHandle<Texture2D> mAlbedoMap;
        ResourceHandle<Texture2D> mMetallicMap;
        ResourceHandle<Texture2D> mRoughnessMap;
        ResourceHandle<Texture2D> mNormalMap;

        // Material properties for this instance
        Float3 mAlbedo;
        f32 mMetallic;
        f32 mRoughness;
        f32 mAo;
        Float3 mEmissive;
        f32 mEmissiveStrength;

    public:
        PBRMaterialInstance() = default;

        void SetBaseMaterial(const shared_ptr<PBRMaterial>& baseMaterial) {
            mBaseMaterial = baseMaterial;
        }

        void SetAlbedo(const Float3& albedo);
        void SetMetallic(f32 metallic);
        void SetRoughness(f32 roughness);
        void SetAO(f32 ao);
        void SetEmissive(const Float3& emissive, f32 strength);

        void SetAlbedoMap(const ResourceHandle<Texture2D>& albedo);
        void SetMetallicMap(const ResourceHandle<Texture2D>& metallic);
        void SetRoughnessMap(const ResourceHandle<Texture2D>& roughness);
        void SetNormalMap(const ResourceHandle<Texture2D>& normal);
        void SetTextureMaps(const ResourceHandle<Texture2D>& albedo,
                            const ResourceHandle<Texture2D>& metallic,
                            const ResourceHandle<Texture2D>& roughness,
                            const ResourceHandle<Texture2D>& normal);

        const PBRMaterial* GetBaseMaterial() const {
            return mBaseMaterial.get();
        }

        void Bind(const TransformMatrices& transforms, const LightState& lights, Float3 eyePos) const;
        void Unbind() const;

    private:
        void UpdateInstanceParams(const TransformMatrices& transforms, const LightState& lights, Float3 eyePos) const;
    };
}  // namespace x