#pragma once

#include "GeometryBuffer.hpp"
#include "InputLayouts.hpp"

namespace x {
    class Mesh {
        GeometryBuffer _geometryBuffer;

    public:
        Mesh(const RenderContext& context, const vector<VSInputPBR>& vertices, const vector<u32>& indices) {
            _geometryBuffer
                .Create(context, vertices.data(), sizeof(VSInputPBR), vertices.size(), indices.data(), indices.size());
        }

        void Draw(RenderContext& context) const {
            _geometryBuffer.Bind(context);
            context.DrawIndexed(_geometryBuffer.GetIndexCount());
        }
    };

    class Model {
        friend class ModelLoader;
        vector<Mesh> _meshes;

    public:
        Model() = default;

        void Draw(RenderContext& context) const {
            for (auto& mesh : _meshes) {
                mesh.Draw(context);
            }
        }
    };

    using ModelHandle = Model*;
}