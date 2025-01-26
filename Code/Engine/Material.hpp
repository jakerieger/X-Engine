#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Lights.hpp"
#include "Shader.hpp"
#include "TransformMatrices.hpp"

namespace x {
    class PBRMaterial {
        Renderer& _renderer;
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
            Float3 emissive;
            f32 emissiveStrength;
        } _materialProperties;

    public:
        explicit PBRMaterial(Renderer& renderer);
        void Apply(const TransformMatrices& transformMatrices, const LightState& lightState, const Float3& cameraPos);

        void SetAlbedo(const Float3& albedo);
        void SetMetallic(f32 metallic);
        void SetRoughness(f32 roughness);
        void SetAO(f32 ao);
        void SetEmissive(const Float3& emissive, f32 strength);

    private:
        void CreateBuffers();
        void UpdateBuffers(const TransformMatrices& transformMatrices,
                           const LightState& lightState,
                           const Float3& cameraPos);
    };
}