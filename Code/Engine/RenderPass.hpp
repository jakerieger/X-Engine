#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Shader.hpp"
#include "Common/Panic.hpp"

namespace x {
    class Renderer;

    class ShadowPass {
        Renderer& _renderer;
        VertexShader _vertexShader;
        PixelShader _pixelShader;
        ComPtr<ID3D11Buffer> _shadowParamsCB;

        struct alignas(16) ShadowMapParams {
            Matrix lightViewProj;
            Matrix world;
        };

        void Initialize();

    public:
        explicit ShadowPass(Renderer& renderer);

        void Begin();
        void End();

        void UpdateParams(const Matrix& lightViewProj, const Matrix& world);
    };

    class LightingPassOpaque {};

    class LightingPassTransparent {};
}