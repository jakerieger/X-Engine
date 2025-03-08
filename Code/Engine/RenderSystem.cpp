#include "RenderSystem.hpp"
#include "RenderContext.hpp"
#include "TonemapEffect.hpp"
#include "Material.hpp"
#include "PBRMaterial.hpp"
#include "ShaderManager.hpp"

#include "ShadowPass_VS.h"
#include "ShadowPass_PS.h"

namespace x {
#pragma region RenderSystem
    RenderSystem::RenderSystem(RenderContext& context, Viewport* viewport)
        : mRenderContext(context), mViewport(viewport), mShadowPass(context), mLightPass(context),
          mPostProcess(context) {}

    void RenderSystem::Initialize() {
        const auto width  = mViewport->GetWidth();
        const auto height = mViewport->GetHeight();

        // Initialize all our render passes
        mShadowPass.Initialize(width, height);
        mLightPass.Initialize(width, height);
        mPostProcess.Initialize(width, height);

        mPostProcess.AddEffect<TonemapEffect>()->SetOperator(TonemapOperator::ACES);
    }

    void RenderSystem::OnResize(u32, u32) {
        const auto width  = mViewport->GetWidth();
        const auto height = mViewport->GetHeight();

        mShadowPass.Resize(width, height);
        mLightPass.Resize(width, height);
        mPostProcess.Resize(width, height);
    }

    void RenderSystem::SetClearColor(float r, float g, float b, float a) {
        mClearColor[0] = r;
        mClearColor[1] = g;
        mClearColor[2] = b;
        mClearColor[3] = a;
    }

    void RenderSystem::BeginShadowPass() {
        mShadowPass.BeginPass();
    }

    ID3D11ShaderResourceView* RenderSystem::EndShadowPass() {
        return mShadowPass.EndPass();
    }

    void RenderSystem::BeginLightPass(ID3D11ShaderResourceView* depthSRV) {
        mLightPass.BeginPass(depthSRV, mClearColor);
    }

    ID3D11ShaderResourceView* RenderSystem::EndLightPass() {
        return mLightPass.EndPass();
    }

    void RenderSystem::PostProcessPass(ID3D11ShaderResourceView* input) {
        const auto& rtv = mViewport->GetRenderTargetView();
        mViewport->BindRenderTarget();
        mPostProcess.Execute(input, rtv.Get());
    }

    void RenderSystem::UpdateShadowPassParameters(const Matrix& lightViewProj, const Matrix& world) {
        mShadowPass.UpdateState({lightViewProj, world});
    }
#pragma endregion

#pragma region ShadowPass
    ShadowPass::ShadowPass(RenderContext& context) : mRenderContext(context) {
        mShader = ShaderManager::GetGraphicsShader(kShadowMapShaderId);
    }

    void ShadowPass::Initialize(u32 width, u32 height) {
        D3D11_BUFFER_DESC desc {};
        desc.ByteWidth      = sizeof(ShadowMapParams);
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        auto hr = mRenderContext.GetDevice()->CreateBuffer(&desc, nullptr, &mShadowParamsCB);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create shadow map constant buffer.")

        Resize(width, height);
    }

    void ShadowPass::BeginPass() {
        mRenderContext.GetDeviceContext()->OMSetRenderTargets(0, nullptr, mDepthStencilView.Get());
        mRenderContext.GetDeviceContext()->OMSetDepthStencilState(mDepthStencilState.Get(), 0);
        mRenderContext.GetDeviceContext()->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        mShader->Bind();

        mRenderContext.GetDeviceContext()->VSSetConstantBuffers(0, 1, mShadowParamsCB.GetAddressOf());
    }

    ID3D11ShaderResourceView* ShadowPass::EndPass() {
        return mDepthSRV.Get();
    }

    void ShadowPass::UpdateState(const ShadowMapParams& state) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        const auto hr =
          mRenderContext.GetDeviceContext()->Map(mShadowParamsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            auto* params          = CAST<ShadowMapParams*>(mapped.pData);
            params->lightViewProj = state.lightViewProj;
            params->world         = state.world;

            mRenderContext.GetDeviceContext()->Unmap(mShadowParamsCB.Get(), 0);
        }
    }

    void ShadowPass::Resize(u32 width, u32 height) {
        mDepthSRV.Reset();
        mDepthStencilState.Reset();
        mDepthStencilView.Reset();

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
        auto hr = mRenderContext.GetDevice()->CreateTexture2D(&depthTexDesc, nullptr, &depthStencilTexture);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil Texture")

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                          = DXGI_FORMAT_R32_FLOAT;  // Format for reading in shader
        srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels             = 1;
        srvDesc.Texture2D.MostDetailedMip       = 0;

        hr = mRenderContext.GetDevice()->CreateShaderResourceView(depthStencilTexture.Get(), &srvDesc, &mDepthSRV);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth SRV")

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice            = 0;

        hr = mRenderContext.GetDevice()->CreateDepthStencilView(depthStencilTexture.Get(),
                                                                &depthStencilViewDesc,
                                                                &mDepthStencilView);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil View")

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
        depthStencilStateDesc.DepthEnable              = TRUE;
        depthStencilStateDesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc                = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable            = FALSE;

        hr = mRenderContext.GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &mDepthStencilState);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil State")
    }
#pragma endregion

#pragma region LightingPass
    void LightPass::Initialize(u32 width, u32 height) {
        auto* device = mRenderContext.GetDevice();

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

        auto hr = device->CreateSamplerState(&comparisonSamplerDesc, &mDepthSamplerState);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create sampler state for shadow pass");

        D3D11_BLEND_DESC blendDesc       = {};
        blendDesc.AlphaToCoverageEnable  = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;

        blendDesc.RenderTarget[0].BlendEnable           = TRUE;
        blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = device->CreateBlendState(&blendDesc, &mBlendState);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create blend state for light pass");

        Resize(width, height);
    }

    void LightPass::BeginPass(ID3D11ShaderResourceView* depthSRV, f32 clearColor[4]) {
        mRenderContext.GetDeviceContext()->OMSetRenderTargets(1,
                                                              mRenderTargetView.GetAddressOf(),
                                                              mDepthStencilView.Get());
        mRenderContext.GetDeviceContext()->OMSetDepthStencilState(mDepthStencilState.Get(), 0);
        mRenderContext.GetDeviceContext()->OMSetBlendState(mBlendState.Get(), nullptr, 0xFFFFFFFF);

        mRenderContext.GetDeviceContext()->ClearRenderTargetView(mRenderTargetView.Get(), clearColor);
        mRenderContext.GetDeviceContext()->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        mRenderContext.GetDeviceContext()->PSSetShaderResources(kShadowMapSlot, 1, &depthSRV);
        mRenderContext.GetDeviceContext()->PSSetSamplers(kShadowMapSlot, 1, mDepthSamplerState.GetAddressOf());
    }

    ID3D11ShaderResourceView* LightPass::EndPass() {
        return mOutputSRV.Get();
    }

    void LightPass::Resize(u32 width, u32 height) {
        mRenderTargetView.Reset();
        mDepthStencilView.Reset();
        mDepthStencilState.Reset();
        mOutputSRV.Reset();
        mSceneTexture.Reset();

        auto* device = mRenderContext.GetDevice();

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

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
        depthStencilStateDesc.DepthEnable              = TRUE;
        depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  // TODO: Change to ALL for opaque pass
        depthStencilStateDesc.DepthFunc      = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable  = FALSE;

        hr = device->CreateDepthStencilState(&depthStencilStateDesc, &mDepthStencilState);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create Depth Stencil State")
        // ==================================================================================================================//
        // ==================================================================================================================//
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

        hr = device->CreateShaderResourceView(mSceneTexture.Get(), nullptr, &mOutputSRV);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create shader resource view")
    }
#pragma endregion
}  // namespace x