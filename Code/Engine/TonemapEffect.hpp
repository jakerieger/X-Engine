#pragma once

#include "ComputeEffect.hpp"

namespace x {
    class TonemapEffect final : public IComputeEffect {
    public:
        explicit TonemapEffect(Renderer& renderer)
            : IComputeEffect(renderer) {}

        bool Initialize() override;

    protected:
        bool CreateResources() override;
        void UpdateConstants() override;
    };
} // namespace x