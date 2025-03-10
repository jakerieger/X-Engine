// Author: Jake Rieger
// Created: 3/10/2025.
//

#include "ShadowPass.hpp"
#include "ShaderManager.hpp"
#include "RenderContext.hpp"
#include "D3D.hpp"

namespace x {
    ShadowPass::ShadowPass(RenderContext& context) : mContext(context) {
        mShader = ShaderManager::GetGraphicsShader(kShadowMapShaderId);
    }

    void ShadowPass::Initialize(u32 width, u32 height) {
        D3D11_BUFFER_DESC desc {};
        desc.ByteWidth      = sizeof(ShadowMapParams);
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        const auto hr = mContext.GetDevice()->CreateBuffer(&desc, nullptr, &mConstantBuffer);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create shadow map constant buffer.")

        Resize(width, height);
    }

    void ShadowPass::BeginPass() {
        auto* context = mContext.GetDeviceContext();
        context->OMSetRenderTargets(0, nullptr, mDepthStencilView.Get());
        context->OMSetDepthStencilState(mDepthStencilState.Get(), 0);
        context->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        mShader->Bind();
        context->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
    }

    void ShadowPass::EndPass(ID3D11ShaderResourceView*& result) const {
        result = mShaderResourceView.Get();
    }

    void ShadowPass::Update(const Matrix& lightViewProj, const Matrix& world) const {
        auto* context = mContext.GetDeviceContext();
        D3D11_MAPPED_SUBRESOURCE mapped;
        const auto hr = context->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            auto* data                 = CAST<ShadowMapParams*>(mapped.pData);
            data->mLightViewProjection = lightViewProj;
            data->mWorld               = world;
            context->Unmap(mConstantBuffer.Get(), 0);
        }
    }

    void ShadowPass::Resize(u32 width, u32 height) {
        mShaderResourceView.Reset();
        mDepthStencilState.Reset();
        mDepthStencilView.Reset();

        auto* device = mContext.GetDevice();

        D3D11_TEXTURE2D_DESC depthTexDesc = {};
        depthTexDesc.Width                = width;
        depthTexDesc.Height               = height;
        depthTexDesc.MipLevels            = 1;
        depthTexDesc.ArraySize            = 1;
        depthTexDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
        depthTexDesc.SampleDesc.Count     = 1;
        depthTexDesc.SampleDesc.Quality   = 0;
        depthTexDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthTexDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        ComPtr<ID3D11Texture2D> depthStencilTexture;
        auto hr = device->CreateTexture2D(&depthTexDesc, nullptr, &depthStencilTexture);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil Texture")

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                          = DXGI_FORMAT_R32_FLOAT;  // Format for reading in shader
        srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels             = 1;
        srvDesc.Texture2D.MostDetailedMip       = 0;

        hr = device->CreateShaderResourceView(depthStencilTexture.Get(), &srvDesc, &mShaderResourceView);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth SRV")

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice            = 0;

        hr = device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &mDepthStencilView);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil View")

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
        depthStencilStateDesc.DepthEnable              = TRUE;
        depthStencilStateDesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc                = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable            = FALSE;

        hr = device->CreateDepthStencilState(&depthStencilStateDesc, &mDepthStencilState);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil State")
    }
}  // namespace x