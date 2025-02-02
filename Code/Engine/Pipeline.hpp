#pragma once

#include "Renderer.hpp"
#include "D3D.hpp"
#include "Common/Types.hpp"
#include "Volatile.hpp"

namespace x {
    class Pipeline final : public Volatile {
    public:
        explicit Pipeline(Renderer& renderer);

        // Prevent moves or copies
        Pipeline(const Pipeline& other)            = delete;
        Pipeline(Pipeline&& other)                 = delete;
        Pipeline& operator=(const Pipeline& other) = delete;
        Pipeline& operator=(Pipeline&& other)      = delete;

        void Initialize();

        void Render();
        void OnResize(u32 width, u32 height) override;

    private:
        Renderer& _renderer;
    };
}