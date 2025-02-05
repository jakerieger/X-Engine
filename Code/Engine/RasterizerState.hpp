#pragma once

#include "RenderContext.hpp"
#include "D3D.hpp"

namespace x {
    class RasterizerStates {
    public:
        static inline ComPtr<ID3D11RasterizerState> DefaultSolid;
        static inline ComPtr<ID3D11RasterizerState> Wireframe;
        static inline ComPtr<ID3D11RasterizerState> NoCull;
        static inline ComPtr<ID3D11RasterizerState> CullFront;
        static inline ComPtr<ID3D11RasterizerState> ScissorTest;

        static void SetupRasterizerStates(const RenderContext& renderer) noexcept;

    private:
        RasterizerStates() = default;
    };
}