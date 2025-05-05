// Author: Jake Rieger
// Created: 2/19/2025.
//

#include "Viewport.hpp"

namespace x {
    bool Viewport::Resize(u32 width, u32 height, bool attachToBackBuffer) {
        // Width and height can never be zero
        width  = (u32)X_MAX(1, (i32)width);
        height = (u32)X_MAX(1, (i32)height);

        mWidth  = width;
        mHeight = height;

        mRenderTarget.Reset();
        mRenderTargetView.Reset();
        mDepthStencilView.Reset();
        mDepthStencilState.Reset();
        mShaderResourceView.Reset();

        mViewport.Width    = CAST<f32>(width);
        mViewport.Height   = CAST<f32>(height);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;
        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;

        auto* ctx    = mContext.GetDeviceContext();
        auto* device = mContext.GetDevice();
        HRESULT hr;
        bool createdRenderTarget = false;

        if (attachToBackBuffer) {
            ID3D11RenderTargetView* nullRTV = nullptr;
            ctx->OMSetRenderTargets(1, &nullRTV, nullptr);
            mContext.ResizeSwapchainBuffers(width, height);
            hr = device->CreateRenderTargetView(mContext.GetBackBuffer(), nullptr, &mRenderTargetView);
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
        hr = device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilTexture);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create depth stencil texture");
            return false;
        }

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice            = 0;

        hr = device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &mDepthStencilView);
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

        hr = device->CreateDepthStencilState(&depthStencilStateDesc, &mDepthStencilState);
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
        sceneDesc.Format           = mFormat;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage            = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        hr = device->CreateTexture2D(&sceneDesc, nullptr, &mRenderTarget);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create render target texture");
            return false;
        }

        hr = device->CreateRenderTargetView(mRenderTarget.Get(), nullptr, &mRenderTargetView);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create render target view");
            return false;
        }

        hr = device->CreateShaderResourceView(mRenderTarget.Get(), nullptr, &mShaderResourceView);
        if (FAILED(hr)) {
            X_LOG_ERROR("Failed to create shader resource view");
            return false;
        }

        return true;
    }

    void Viewport::AttachViewport() const {
        mContext.GetDeviceContext()->RSSetViewports(1, &mViewport);
    }

    void Viewport::DetachViewport() const {
        mContext.GetDeviceContext()->RSSetViewports(1, nullptr);
    }

    void Viewport::ClearRenderTargetView() const {
        auto* ctx = mContext.GetDeviceContext();
        ctx->ClearRenderTargetView(mRenderTargetView.Get(), mClearColor);
    }

    void Viewport::ClearDepthStencilView(f32 depth, u8 stencil) const {
        auto* ctx = mContext.GetDeviceContext();
        ctx->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
    }

    void Viewport::ClearAll(f32 depth, u8 stencil) const {
        ClearRenderTargetView();
        ClearDepthStencilView(depth, stencil);
    }

    void Viewport::BindRenderTarget() const {
        auto* ctx = mContext.GetDeviceContext();
        ctx->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
        ctx->OMSetDepthStencilState(mDepthStencilState.Get(), 0);
    }

    void Viewport::SetClearColor(const Color& color) {
        color.ToFloatArray(mClearColor);
    }

    ComPtr<ID3D11RenderTargetView> const& Viewport::GetRenderTargetView() {
        return mRenderTargetView;
    }

    ComPtr<ID3D11DepthStencilView> const& Viewport::GetDepthStencilView() {
        return mDepthStencilView;
    }

    ComPtr<ID3D11DepthStencilState> const& Viewport::GetDepthStencilState() {
        return mDepthStencilState;
    }

    ComPtr<ID3D11ShaderResourceView> const& Viewport::GetShaderResourceView() {
        return mShaderResourceView;
    }

    D3D11_VIEWPORT Viewport::GetViewport() const {
        return mViewport;
    }

    u32 Viewport::GetWidth() const {
        return mWidth;
    }

    u32 Viewport::GetHeight() const {
        return mHeight;
    }
}  // namespace x