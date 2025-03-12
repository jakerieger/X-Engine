// Author: Jake Rieger
// Created: 3/10/2025.
//

#include "LightPass.hpp"
#include "RenderContext.hpp"
#include "D3D.hpp"

namespace x {
    void LightPass::Initialize(u32 width, u32 height) {
        auto* device = mContext.GetDevice();

        D3D11_SAMPLER_DESC comparisonSamplerDesc {};
        comparisonSamplerDesc.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        comparisonSamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
        comparisonSamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
        comparisonSamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
        comparisonSamplerDesc.BorderColor[0] = 1.0f;  // Use 1.0f for all components
        comparisonSamplerDesc.BorderColor[1] = 1.0f;  // This means "not in shadow" for
        comparisonSamplerDesc.BorderColor[2] = 1.0f;  // areas outside the shadow map
        comparisonSamplerDesc.BorderColor[3] = 1.0f;
        comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;  // Key setting!

        auto hr = device->CreateSamplerState(&comparisonSamplerDesc, &mShadowMapSamplerState);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create sampler state for shadow pass");

        // Opaque blend state
        D3D11_BLEND_DESC opaqueDesc                      = {};
        opaqueDesc.AlphaToCoverageEnable                 = FALSE;
        opaqueDesc.IndependentBlendEnable                = FALSE;
        opaqueDesc.RenderTarget[0].BlendEnable           = FALSE;
        opaqueDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
        opaqueDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_ZERO;
        opaqueDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        opaqueDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        opaqueDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
        opaqueDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        opaqueDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = device->CreateBlendState(&opaqueDesc, &mBlendStateOpaque);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create opaque blend state for light pass");

        // Transparent blend state
        D3D11_BLEND_DESC transparentDesc                      = {};
        transparentDesc.AlphaToCoverageEnable                 = FALSE;
        transparentDesc.IndependentBlendEnable                = FALSE;
        transparentDesc.RenderTarget[0].BlendEnable           = TRUE;
        transparentDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        transparentDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        transparentDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        transparentDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        transparentDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
        transparentDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = device->CreateBlendState(&transparentDesc, &mBlendStateTransparent);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create transparent blend state for light pass");

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC opaqueDepthStencilDesc = {};
        opaqueDepthStencilDesc.DepthEnable              = TRUE;              // Enable depth testing
        opaqueDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  // Write to the entire depth buffer
        opaqueDepthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;  // Standard depth test (smaller Z is closer)

        // Stencil settings - often not used for basic opaque pass
        opaqueDepthStencilDesc.StencilEnable    = FALSE;  // Disable stencil testing
        opaqueDepthStencilDesc.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;
        opaqueDepthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        // Front face stencil operations
        opaqueDepthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        opaqueDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        opaqueDepthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        opaqueDepthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

        // Back face stencil operations (same as front face in this case)
        opaqueDepthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        opaqueDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        opaqueDepthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        opaqueDepthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

        hr = device->CreateDepthStencilState(&opaqueDepthStencilDesc, &mDepthStencilStateOpaque);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create opaque Depth Stencil State")

        D3D11_DEPTH_STENCIL_DESC transparentDepthStencilDesc = {};
        transparentDepthStencilDesc.DepthEnable              = TRUE;               // Enable depth testing
        transparentDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // Don't write to depth buffer
        transparentDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;  // Less-equal allows rendering on same Z

        // Stencil settings (typically not used for basic transparency)
        transparentDepthStencilDesc.StencilEnable    = FALSE;  // Disable stencil testing
        transparentDepthStencilDesc.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;
        transparentDepthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        // Front face stencil operations
        transparentDepthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        transparentDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        transparentDepthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        transparentDepthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

        // Back face stencil operations (same as front face in this case)
        transparentDepthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
        transparentDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        transparentDepthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
        transparentDepthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

        hr = device->CreateDepthStencilState(&transparentDepthStencilDesc, &mDepthStencilStateTransparent);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create transparent Depth Stencil State")

        Resize(width, height);
    }

    void LightPass::BeginPass(ID3D11ShaderResourceView* depthMap, f32 clearColor[4]) {
        auto* context = mContext.GetDeviceContext();
        context->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
        context->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);
        context->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        context->PSSetShaderResources(5, 1, &depthMap);
        context->PSSetSamplers(5, 1, mShadowMapSamplerState.GetAddressOf());
    }

    void LightPass::EndPass(ID3D11ShaderResourceView*& result) const {
        result = mShaderResourceView.Get();
    }

    void LightPass::OpaqueState() const {
        mContext.GetDeviceContext()->OMSetDepthStencilState(mDepthStencilStateOpaque.Get(), 0);
        mContext.GetDeviceContext()->OMSetBlendState(mBlendStateOpaque.Get(), nullptr, 0xFFFFFFFF);
    }

    void LightPass::TransparentState() const {
        mContext.GetDeviceContext()->OMSetDepthStencilState(mDepthStencilStateTransparent.Get(), 0);
        mContext.GetDeviceContext()->OMSetBlendState(mBlendStateTransparent.Get(), nullptr, 0xFFFFFFFF);
    }

    void LightPass::Resize(u32 width, u32 height) {
        mRenderTargetView.Reset();
        mDepthStencilView.Reset();
        mShaderResourceView.Reset();
        mSceneTexture.Reset();

        auto* device = mContext.GetDevice();

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
        auto hr = device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilTexture);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil Texture")

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice            = 0;

        hr = device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &mDepthStencilView);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil View")

        D3D11_TEXTURE2D_DESC sceneDesc {};
        sceneDesc.Width            = width;
        sceneDesc.Height           = height;
        sceneDesc.MipLevels        = 1;
        sceneDesc.ArraySize        = 1;
        sceneDesc.Format           = DXGI_FORMAT_R16G16B16A16_FLOAT;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage            = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        hr = device->CreateTexture2D(&sceneDesc, nullptr, &mSceneTexture);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create scene texture")

        hr = device->CreateRenderTargetView(mSceneTexture.Get(), nullptr, &mRenderTargetView);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create render target view.");

        hr = device->CreateShaderResourceView(mSceneTexture.Get(), nullptr, &mShaderResourceView);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create shader resource view")
    }
}  // namespace x