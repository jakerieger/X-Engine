// Author: Jake Rieger
// Created: 2/19/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "RenderContext.hpp"

namespace x {
    class Viewport {
    public:
        explicit Viewport(RenderContext& context, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
            : _format(format), _context(context), _width(0), _height(0) {}

        bool Resize(u32 width, u32 height, bool attachToBackBuffer = false);
        void AttachViewport() const;
        void DetachViewport() const;

        void ClearRenderTargetView(const f32 clearColor[4] = Colors::CornflowerBlue) const;
        void ClearDepthStencilView(f32 depth = 1.0f, u8 stencil = 0) const;
        void ClearAll(const f32 clearColor[4] = Colors::CornflowerBlue, f32 depth = 1.0f, u8 stencil = 0) const;
        void BindRenderTarget() const;

        X_NODISCARD ComPtr<ID3D11RenderTargetView> const& GetRenderTargetView();
        X_NODISCARD ComPtr<ID3D11DepthStencilView> const& GetDepthStencilView();
        X_NODISCARD ComPtr<ID3D11DepthStencilState> const& GetDepthStencilState();
        X_NODISCARD ComPtr<ID3D11ShaderResourceView> const& GetShaderResourceView();
        X_NODISCARD D3D11_VIEWPORT GetViewport() const;
        X_NODISCARD u32 GetWidth() const;
        X_NODISCARD u32 GetHeight() const;

    private:
        DXGI_FORMAT _format;
        D3D11_VIEWPORT _viewport {};
        RenderContext& _context;
        u32 _width, _height;
        ComPtr<ID3D11Texture2D> _renderTarget;
        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;
        ComPtr<ID3D11ShaderResourceView> _shaderResourceView;
    };
}  // namespace x
