#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Common/Panic.hpp"

namespace x {
    class Renderer;

    // TODO: Support resizing
    class ShadowPass {
        Renderer& _renderer;
        VertexShader _vertexShader;
        PixelShader _pixelShader;
        ComPtr<ID3D11Buffer> _shadowParamsCB;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;
        ComPtr<ID3D11ShaderResourceView> _depthSRV;
        vector<ModelHandle> _occluders;

        struct alignas(16) ShadowMapParams {
            Matrix lightViewProj;
            Matrix world;
        };

    public:
        explicit ShadowPass(Renderer& renderer);
        void Initialize(u32 width, u32 height);
        void Draw();

        void UpdateParams(const Matrix& lightViewProj, const Matrix& world);
        void AddOccluder(const ModelHandle& occluder);

        [[nodiscard]] ID3D11ShaderResourceView* GetDepthSRV() const {
            return _depthSRV.Get();
        }
    };

    class LightingPass {
        Renderer& _renderer;
        vector<ModelHandle> _opaqueMeshes;
        vector<ModelHandle> _transparentMeshes;

    public:
        LightingPass(Renderer& renderer) : _renderer(renderer) {}
    };
}