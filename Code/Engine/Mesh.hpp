#pragma once

#include "Common/Types.hpp"
#include "GeometryBuffer.hpp"
#include "InputLayouts.hpp"
#include "Material.hpp"

namespace x {
    class Mesh {
        RenderContext& _renderer;
        GeometryBuffer _geometryBuffers{};

    public:
        Mesh(RenderContext& renderer, const vector<VSInputPBR>& vertices, const vector<u32>& indices);
        void Draw();
    };
}