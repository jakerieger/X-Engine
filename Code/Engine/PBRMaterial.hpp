// Author: Jake Rieger
// Created: 2/28/2025.
//

#pragma once

#include "Lights.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "TransformMatrices.hpp"

// PBR Material Parameters
//
// World:
// ----------------
// Transform Matrices
// Direct Lights (Directional, Spot, Area, Point)
// Indirect (Reflection Probes, IBL)
// Camera Position
//
// Scalar:
// ----------------
// Albedo -> Float3
// Metallic -> Float
// Roughness -> Float
// AO -> Float
//
// Texture Maps:
// ----------------
// AlbedoMap
// MetallicMap
// RoughnessMap
// AOMap
// NormalMap
// ShadowMap
//
// IBL
// ----------------
// RadianceMap
// IrradianceMap
// BRDF LUT

namespace x {
    static constexpr u32 kAlbedoMapSlot           = 0;
    static constexpr u32 kMetallicMapSlot         = 1;
    static constexpr u32 kRoughnessMapSlot        = 2;
    static constexpr u32 kAmbientOcclusionMapSlot = 3;
    static constexpr u32 kNormalMapSlot           = 4;
    static constexpr u32 kShadowMapSlot           = 5;

    class PBRMaterial final : public IMaterial {
    public:
        explicit PBRMaterial(RenderContext& context, bool transparent = false);

        void Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const override;
        void Unbind() const override;

        void SetAlbedoMap(const ResourceHandle<Texture2D>& albedo);
        void SetMetallicMap(const ResourceHandle<Texture2D>& metallic);
        void SetRoughnessMap(const ResourceHandle<Texture2D>& roughness);
        void SetNormalMap(const ResourceHandle<Texture2D>& normal);
        void SetTextureMaps(const ResourceHandle<Texture2D>& albedo,
                            const ResourceHandle<Texture2D>& metallic,
                            const ResourceHandle<Texture2D>& roughness,
                            const ResourceHandle<Texture2D>& normal);

    protected:
        MaterialBuffers mBuffers;
        ResourceHandle<Texture2D> mAlbedoMap;
        ResourceHandle<Texture2D> mMetallicMap;
        ResourceHandle<Texture2D> mRoughnessMap;
        ResourceHandle<Texture2D> mNormalMap;

        void CreateBuffers() override;
        void UpdateBuffers(const MaterialParameters& params) const override;
        void BindBuffers() const override;
    };
}  // namespace x
