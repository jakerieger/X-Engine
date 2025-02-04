#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
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

        struct alignas(16) ShadowMapParams {
            Matrix lightViewProj;
            Matrix world;
        };

    public:
        explicit ShadowPass(Renderer& renderer);
        void Initialize(u32 width, u32 height);

        void Begin();
        void End();

        void UpdateParams(const Matrix& lightViewProj, const Matrix& world);
    };

    class LightingPassOpaque {};

    class LightingPassTransparent {};
}