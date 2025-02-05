#pragma once

#include "ComputeEffect.hpp"
#include "D3D.hpp"
#include "Common/Types.hpp"
#include "Volatile.hpp"

namespace x {
    class RenderContext;

    struct RenderTarget {
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11UnorderedAccessView> uav;
        ComPtr<ID3D11ShaderResourceView> srv;

        void Reset() {
            texture.Reset();
            uav.Reset();
            srv.Reset();
        }
    };

    class PostProcessSystem final : public Volatile {
        RenderContext& _renderer;
        VertexShader _fullscreenVS;
        PixelShader _fullscreenPS;
        u32 _width  = 0;
        u32 _height = 0;
        vector<unique_ptr<IComputeEffect>> _effects;
        vector<RenderTarget> _intermediateTargets;

        bool CreateIntermediateTargets();
        void RenderToScreen(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);

    public:
        explicit PostProcessSystem(RenderContext& renderer) : _renderer(renderer), _fullscreenVS(renderer),
                                                              _fullscreenPS(renderer) {}

        bool Initialize(u32 width, u32 height);
        void OnResize(u32 width, u32 height) override;
        void Execute(ID3D11ShaderResourceView* sceneInput, ID3D11RenderTargetView* finalOutput);

        template<typename T, typename... Args>
            requires std::is_base_of_v<IComputeEffect, T>
        T* AddEffect(Args&&... args) {
            auto effect = make_unique<T>(_renderer, std::forward<Args>(args)...);

            if (!effect->Initialize()) {
                return None;
            }

            effect->OnResize(_width, _height);

            T* ptr = effect.get();
            _effects.push_back(std::move(effect));
            CreateIntermediateTargets();

            return ptr;
        }
    };
}