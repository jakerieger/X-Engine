#pragma once

#include "Common/Types.hpp"
#include "Renderer.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "TransformComponent.hpp"

namespace x {
    class ModelData;
    class ModelHandle;

    class ModelHandle final : public Resource {
        shared_ptr<ModelData> _modelData;

    public:
        ModelHandle() = default;

        void Draw();
        void Release() override;
        [[nodiscard]] bool Valid() const;

        void SetModelData(const shared_ptr<ModelData>& modelData) {
            _modelData = modelData;
        }
    };

    class ModelData {
        friend ModelHandle;
        friend class FBXLoader;
        friend class GenericLoader;

    public:
        explicit ModelData(Renderer& renderer) : _renderer(renderer) {}
        [[nodiscard]] bool Valid() const;

    private:
        Renderer& _renderer;
        vector<unique_ptr<Mesh>> _meshes;

        void Draw();
    };
}