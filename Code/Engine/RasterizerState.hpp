#pragma once

#include "Renderer.hpp"
#include "D3D.hpp"

namespace x {
    class RasterizerStates {
    public:
        static inline ComPtr<ID3D11RasterizerState> DefaultSolid;
        static inline ComPtr<ID3D11RasterizerState> Wireframe;
        static inline ComPtr<ID3D11RasterizerState> NoCull;
        static inline ComPtr<ID3D11RasterizerState> CullFront;
        static inline ComPtr<ID3D11RasterizerState> ScissorTest;

        static void SetupRasterizerStates(const Renderer& renderer) noexcept;

    private:
        RasterizerStates() = default;
    };
}