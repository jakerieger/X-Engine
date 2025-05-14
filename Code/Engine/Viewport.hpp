// Author: Jake Rieger
// Created: 2/19/2025.
//

#pragma once

#include "Color.hpp"
#include "Common/Typedefs.hpp"
#include "D3D.hpp"
#include "RenderContext.hpp"

namespace x {
    class Viewport {
    public:
        explicit Viewport(RenderContext& context, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
            : mFormat(format), mContext(context), mWidth(0), mHeight(0) {}

        bool Resize(u32 width, u32 height, bool attachToBackBuffer = false);
        void AttachViewport() const;
        void DetachViewport() const;

        void ClearRenderTargetView() const;
        void ClearDepthStencilView(f32 depth = 1.0f, u8 stencil = 0) const;
        void ClearAll(f32 depth = 1.0f, u8 stencil = 0) const;
        void BindRenderTarget() const;

        void SetClearColor(const Color& color);

        X_NODISCARD ComPtr<ID3D11RenderTargetView> const& GetRenderTargetView();
        X_NODISCARD ComPtr<ID3D11DepthStencilView> const& GetDepthStencilView();
        X_NODISCARD ComPtr<ID3D11DepthStencilState> const& GetDepthStencilState();
        X_NODISCARD ComPtr<ID3D11ShaderResourceView> const& GetShaderResourceView();
        X_NODISCARD D3D11_VIEWPORT GetViewport() const;
        X_NODISCARD u32 GetWidth() const;
        X_NODISCARD u32 GetHeight() const;

    private:
        DXGI_FORMAT mFormat;
        D3D11_VIEWPORT mViewport {};
        RenderContext& mContext;
        u32 mWidth = 1, mHeight = 1;
        f32 mClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        ComPtr<ID3D11Texture2D> mRenderTarget;
        ComPtr<ID3D11RenderTargetView> mRenderTargetView;
        ComPtr<ID3D11DepthStencilView> mDepthStencilView;
        ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        ComPtr<ID3D11ShaderResourceView> mShaderResourceView;
    };
}  // namespace x
