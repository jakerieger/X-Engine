#pragma once

#include "Common/Types.hpp"
#include "RenderContext.hpp"
#include "Mesh.hpp"

namespace x {
    class ModelData;
    class ModelHandle;

    class ModelHandle final {
        shared_ptr<ModelData> _modelData;

    public:
        ModelHandle() = default;

        void Draw() const;
        bool Valid() const;
        size_t NumMeshes() const;

        void SetModelData(const shared_ptr<ModelData>& modelData) {
            _modelData = modelData;
        }
    };

    class ModelData {
        friend ModelHandle;
        friend class FBXLoader;
        friend class GenericLoader;
        friend class ModelLoader;

    public:
        explicit ModelData(RenderContext& renderer) : _renderer(renderer) {}
        bool Valid() const;

    private:
        RenderContext& _renderer;
        vector<unique_ptr<Mesh>> _meshes;

        void Draw();
    };
}