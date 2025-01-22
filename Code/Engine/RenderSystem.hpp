#pragma once

#include "Renderer.hpp"
#include "D3D.hpp"
#include "Common/Types.hpp"
#include "Volatile.hpp"

namespace x {
    class RenderSystem final : public Volatile {
    public:
        explicit RenderSystem(Renderer& renderer);

        // Prevent moves or copies
        RenderSystem(const RenderSystem& other)            = delete;
        RenderSystem(RenderSystem&& other)                 = delete;
        RenderSystem& operator=(const RenderSystem& other) = delete;
        RenderSystem& operator=(RenderSystem&& other)      = delete;

        void Initialize();

        void Render();
        void OnResize(u32 width, u32 height) override;

    private:
        Renderer& _renderer;
    };
}