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
        TextureHandle<Texture2D> _albedoMap;
        TextureHandle<Texture2D> _metallicMap;
        TextureHandle<Texture2D> _roughnessMap;
        TextureHandle<Texture2D> _normalMap;

        struct alignas(16) MaterialProperties {
            Float3 albedo;
            f32 metallic;
            f32 roughness;
            f32 ao;
            Float3 emissive;
            f32 emissiveStrength;
        } _materialProperties;

    public:
        explicit PBRMaterial(RenderContext& renderer);
        void Apply();
        void Clear();

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

        void UpdateBuffers(const TransformMatrices& transforms,
                           const LightState& lights,
                           const Float3& eyePosition);

        static shared_ptr<PBRMaterial> Create(RenderContext& renderer);

    private:
        void CreateBuffers();
    };
}