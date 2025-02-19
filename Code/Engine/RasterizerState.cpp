#include "RasterizerState.hpp"

namespace x {
    void RasterizerStates::Initialize(const RenderContext& renderer) noexcept {
        D3D11_RASTERIZER_DESC defaultSolid{};
        defaultSolid.FillMode              = D3D11_FILL_SOLID;
        defaultSolid.CullMode              = D3D11_CULL_BACK;
        defaultSolid.FrontCounterClockwise = FALSE;
        defaultSolid.DepthClipEnable       = TRUE;

        D3D11_RASTERIZER_DESC wireframe = defaultSolid;
        wireframe.FillMode              = D3D11_FILL_WIREFRAME;

        D3D11_RASTERIZER_DESC noCull = defaultSolid;
        noCull.CullMode              = D3D11_CULL_NONE;

        D3D11_RASTERIZER_DESC cullFront = defaultSolid;
        cullFront.CullMode              = D3D11_CULL_FRONT;

        D3D11_RASTERIZER_DESC scissorTest = defaultSolid;
        scissorTest.ScissorEnable         = TRUE;

        auto* device = renderer.GetDevice();
        auto hr      = device->CreateRasterizerState(&defaultSolid, &DefaultSolid);
        X_PANIC_ASSERT(SUCCEEDED(hr),
                       "Failed to create DefaultSolid rasterizer state.");

        hr = device->CreateRasterizerState(&wireframe, &Wireframe);
        X_PANIC_ASSERT(SUCCEEDED(hr),
                       "Failed to create Wireframe rasterizer state.");

        hr = device->CreateRasterizerState(&noCull, &NoCull);
        X_PANIC_ASSERT(SUCCEEDED(hr),
                       "Failed to create NoCull rasterizer state.");

        hr = device->CreateRasterizerState(&cullFront, &CullFront);
        X_PANIC_ASSERT(SUCCEEDED(hr),
                       "Failed to create CullFront rasterizer state.");

        hr = device->CreateRasterizerState(&scissorTest, &ScissorTest);
        X_PANIC_ASSERT(SUCCEEDED(hr),
                       "Failed to create ScissorTest rasterizer state.");
    }
}