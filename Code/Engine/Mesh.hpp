#pragma once

#include "Common/Types.hpp"
#include "GeometryBuffer.hpp"
#include "InputLayouts.hpp"

namespace x {
    class Mesh {
        Renderer& _renderer;
        GeometryBuffer<VSInputPosTexNormal> _geometryBuffers{};

    public:
        Mesh(Renderer& renderer, const vector<VSInputPosTexNormal>& vertices, const vector<u32>& indices);
        ~Mesh();

        void Draw();
    };
}