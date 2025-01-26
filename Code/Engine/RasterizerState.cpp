#include "RasterizerState.hpp"

namespace x {
    void RasterizerStates::SetupRasterizerStates(const Renderer& renderer) {
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
        DX_THROW_IF_FAILED(device->CreateRasterizerState(&defaultSolid, &DefaultSolid));
        DX_THROW_IF_FAILED(device->CreateRasterizerState(&wireframe, &Wireframe));
        DX_THROW_IF_FAILED(device->CreateRasterizerState(&noCull, &NoCull));
        DX_THROW_IF_FAILED(device->CreateRasterizerState(&cullFront, &CullFront));
        DX_THROW_IF_FAILED(device->CreateRasterizerState(&scissorTest, &ScissorTest));
    }
}