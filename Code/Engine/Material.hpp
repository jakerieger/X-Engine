#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Lights.hpp"
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
        RenderContext& _renderer;
        ComPtr<ID3D11Buffer> _transformsCB;
        ComPtr<ID3D11Buffer> _lightsCB;
        ComPtr<ID3D11Buffer> _materialCB;
        ComPtr<ID3D11Buffer> _cameraCB;
        unique_ptr<VertexShader> _vertexShader;
        unique_ptr<PixelShader> _pixelShader;

        struct alignas(16) MaterialProperties {
            Float3 albedo;
            f32 metallic;
            f32 roughness;
            f32 ao;
            f32 _pad[2];
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
        shared_ptr<PBRMaterial> _baseMaterial;

        // Texture maps for this instance
        TextureHandle<Texture2D> _albedoMap;
        TextureHandle<Texture2D> _metallicMap;
        TextureHandle<Texture2D> _roughnessMap;
        TextureHandle<Texture2D> _normalMap;

        // Material properties for this instance
        Float3 _albedo;
        f32 _metallic;
        f32 _roughness;
        f32 _ao;
        Float3 _emissive;
        f32 _emissiveStrength;

    public:
        PBRMaterialInstance() = default;

        void SetBaseMaterial(const shared_ptr<PBRMaterial>& baseMaterial) {
            _baseMaterial = baseMaterial;
        }

        void SetAlbedo(const Float3& albedo);
        void SetMetallic(f32 metallic);
        void SetRoughness(f32 roughness);
        void SetAO(f32 ao);
        void SetEmissive(const Float3& emissive, f32 strength);

        void SetAlbedoMap(const TextureHandle<Texture2D>& albedo);
        void SetMetallicMap(const TextureHandle<Texture2D>& metallic);
        void SetRoughnessMap(const TextureHandle<Texture2D>& roughness);
        void SetNormalMap(const TextureHandle<Texture2D>& normal);
        void SetTextureMaps(const TextureHandle<Texture2D>& albedo,
                            const TextureHandle<Texture2D>& metallic,
                            const TextureHandle<Texture2D>& roughness,
                            const TextureHandle<Texture2D>& normal);

        const PBRMaterial* GetBaseMaterial() const {
            return _baseMaterial.get();
        }

        void Bind(const TransformMatrices& transforms, const LightState& lights, Float3 eyePos) const;
        void Unbind() const;

    private:
        void UpdateInstanceParams(const TransformMatrices& transforms, const LightState& lights, Float3 eyePos) const;
    };
}