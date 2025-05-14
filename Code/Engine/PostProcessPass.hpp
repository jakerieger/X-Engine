#pragma once

#include "ComputeEffect.hpp"
#include "D3D.hpp"
#include "Common/Typedefs.hpp"

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

    class PostProcessPass {
        RenderContext& mRenderer;
        VertexShader mFullscreenVS;
        PixelShader mFullscreenPS;
        u32 mWidth  = 0;
        u32 mHeight = 0;
        vector<unique_ptr<IComputeEffect>> mEffects;
        vector<RenderTarget> mIntermediateTargets;

        bool CreateIntermediateTargets();
        void RenderToScreen(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);

    public:
        explicit PostProcessPass(RenderContext& renderer)
            : mRenderer(renderer), mFullscreenVS(renderer), mFullscreenPS(renderer) {}

        bool Initialize(u32 width, u32 height);
        void Resize(u32 width, u32 height);
        void Execute(ID3D11ShaderResourceView* sceneInput, ID3D11RenderTargetView* finalOutput);

        template<typename T, typename... Args>
            requires std::is_base_of_v<IComputeEffect, T>
        T* AddEffect(Args&&... args) {
            auto effect = make_unique<T>(mRenderer, std::forward<Args>(args)...);

            if (!effect->Initialize()) { return nullptr; }

            effect->OnResize(mWidth, mHeight);

            T* ptr = effect.get();
            mEffects.push_back(std::move(effect));
            CreateIntermediateTargets();

            return ptr;
        }

        template<typename T>
        T* GetEffect() {
            for (auto& effect : mEffects) {
                T* ptr = effect->As<T>();
                if (ptr) return ptr;
            }
            return nullptr;
        }
    };
}  // namespace x