#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Lights.hpp"
#include "Material.hpp"
#include "Volatile.hpp"
#include "Common/Panic.hpp"

namespace x {
    class Renderer;

    struct ShadowPassState {
        Matrix lightViewProj;
        Matrix world;
    };

    struct LightingPassState {
        TransformMatrices transforms;
        LightState lights;
        Float3 eyePosition;
    };

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

        void UpdateState(const ShadowPassState& state);
        void AddOccluder(const ModelHandle& occluder);

        [[nodiscard]] ID3D11ShaderResourceView* GetDepthSRV() const {
            return _depthSRV.Get();
        }
    };

    class LightingPass {
        Renderer& _renderer;
        vector<ModelHandle> _opaqueObjects;
        vector<ModelHandle> _transparentObjects;
        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;

    public:
        explicit LightingPass(Renderer& renderer) : _renderer(renderer) {}
        void Initialize(u32 width, u32 height);
        void Draw(ID3D11ShaderResourceView* depthSRV);

        void UpdateState(const LightingPassState& state);
        void AddOpaqueObject(const ModelHandle& object);
        void AddTransparentObject(const ModelHandle& object);
    };

    class RenderSystem final : public Volatile {
    public:
        explicit RenderSystem(Renderer& renderer);
        void Initialize(u32 width, u32 height);

        void DrawShadowPass();
        void DrawLightingPass();

        void UpdateShadowParams(const LightState& lights);
        void UpdateLightingParams();

        void RegisterOccluder(const ModelHandle& occluder);
        void RegisterOpaqueObject(const ModelHandle& object);
        void RegisterTransparentObject(const ModelHandle& object);

        template<typename... Args>
            requires (std::is_same_v<ModelHandle, Args> && ...)
        void RegisterOccluders(const Args&... args) {
            (RegisterOccluder(args), ...);
        }

        template<typename... Args>
            requires(std::is_same_v<ModelHandle, Args> && ...)
        void RegisterOpaqueObjects(const Args&... args) {
            (RegisterOpaqueObject(args), ...);
        }

        template<typename... Args>
            requires(std::is_same_v<ModelHandle, Args> && ...)
        void RegisterTransparentObjects(const Args&... args) {
            (RegisterTransparentObject(args), ...);
        }

        void OnResize(u32 width, u32 height) override;

    private:
        ShadowPass _shadowPass;
        LightingPass _lightingPass;
        u32 _width  = 0;
        u32 _height = 0;
    };
}