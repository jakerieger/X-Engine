// Author: Jake Rieger
// Created: 2/19/2025.
//

#include "Viewport.hpp"

namespace x {
    bool Viewport::Resize(u32 width, u32 height, bool attachToBackBuffer) {
        X_DEBUG_LOG_RESIZE("Viewport", width, height)

        _width  = width;
        _height = height;

        _renderTarget.Reset();
        _renderTargetView.Reset();
        _depthStencilView.Reset();
        _depthStencilState.Reset();
        _shaderResourceView.Reset();

        _viewport.Width    = CAST<f32>(width);
        _viewport.Height   = CAST<f32>(height);
        _viewport.MinDepth = 0.0f;
        _viewport.MaxDepth = 1.0f;
        _viewport.TopLeftX = 0.0f;
        _viewport.TopLeftY = 0.0f;

        auto* ctx    = _context.GetDeviceContext();
        auto* device = _context.GetDevice();
        HRESULT hr;
        bool createdRenderTarget = false;

        if (attachToBackBuffer) {
            ID3D11RenderTargetView* nullRTV = nullptr;
            ctx->OMSetRenderTargets(1, &nullRTV, nullptr);
            _context.ResizeSwapchainBuffers(width, height);
            hr = device->CreateRenderTargetView(_context.GetBackBuffer(), None, &_renderTargetView);
            if (FAILED(hr)) {
                X_LOG_ERROR("Failed to create render target");
                return false;
            }
            createdRenderTarget = true;
        }

        D3D11_TEXTURE2D_DESC depthStencilDesc = {};
        depthStencilDesc.Width                = width;
        depthStencilDesc.Height               = height;
        depthStencilDesc.MipLevels            = 1;
        depthStencilDesc.ArraySize            = 1;
        depthStencilDesc.Format               = DXGI_FORMAT_R24G8_TYPELESS;
        depthStencilDesc.SampleDesc.Count     = 1;
        depthStencilDesc.SampleDesc.Quality   = 0;
        depthStencilDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        ComPtr<ID3D11Texture2D> depthStencilTexture;
        hr = device->CreateTexture2D(&depthStencilDesc, None, &depthStencilTexture);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create depth stencil texture");
            return false;
        }

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice            = 0;

        hr = device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &_depthStencilView);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create depth stencil view");
            return false;
        }

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
        depthStencilStateDesc.DepthEnable              = TRUE;
        depthStencilStateDesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc                = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable            = FALSE;

        hr = device->CreateDepthStencilState(&depthStencilStateDesc, &_depthStencilState);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create depth stencil state");
            return false;
        }

        if (createdRenderTarget) { return true; }

        D3D11_TEXTURE2D_DESC sceneDesc {};
        sceneDesc.Width            = width;
        sceneDesc.Height           = height;
        sceneDesc.MipLevels        = 1;
        sceneDesc.ArraySize        = 1;
        sceneDesc.Format           = _format;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage            = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        hr = device->CreateTexture2D(&sceneDesc, None, &_renderTarget);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create render target texture");
            return false;
        }

        hr = device->CreateRenderTargetView(_renderTarget.Get(), None, &_renderTargetView);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create render target view");
            return false;
        }

        hr = device->CreateShaderResourceView(_renderTarget.Get(), None, &_shaderResourceView);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create shader resource view");
            return false;
        }

        return true;
    }

    void Viewport::AttachViewport() const {
        _context.GetDeviceContext()->RSSetViewports(1, &_viewport);
    }

    void Viewport::DetachViewport() const {
        _context.GetDeviceContext()->RSSetViewports(1, None);
    }

    void Viewport::ClearRenderTargetView(const f32 clearColor[4]) const {
        auto* ctx = _context.GetDeviceContext();
        ctx->ClearRenderTargetView(_renderTargetView.Get(), clearColor);
    }

    void Viewport::ClearDepthStencilView(f32 depth, u8 stencil) const {
        auto* ctx = _context.GetDeviceContext();
        ctx->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    void Viewport::BindRenderTarget() const {
        auto* ctx = _context.GetDeviceContext();
        ctx->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
        ctx->OMSetDepthStencilState(_depthStencilState.Get(), 0);
    }

    ComPtr<ID3D11RenderTargetView> const& Viewport::GetRenderTargetView() {
        return _renderTargetView;
    }

    ComPtr<ID3D11DepthStencilView> const& Viewport::GetDepthStencilView() {
        return _depthStencilView;
    }

    ComPtr<ID3D11DepthStencilState> const& Viewport::GetDepthStencilState() {
        return _depthStencilState;
    }

    ComPtr<ID3D11ShaderResourceView> const& Viewport::GetShaderResourceView() {
        return _shaderResourceView;
    }

    D3D11_VIEWPORT Viewport::GetViewport() const {
        return _viewport;
    }

    u32 Viewport::GetWidth() const {
        return _width;
    }

    u32 Viewport::GetHeight() const {
        return _height;
    }
}  // namespace x