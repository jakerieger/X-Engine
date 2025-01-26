#include "Model.hpp"

namespace x {
    #pragma region ModelHandle
    void ModelHandle::Draw(const Camera& camera, const TransformComponent& transform) {
        if (_modelData) {
            _modelData->Draw(camera, transform);
        }
    }

    void ModelHandle::Release() {
        _modelData.reset();
    }

    bool ModelHandle::Valid() const {
        return _modelData && _modelData->Valid();
    }
    #pragma endregion

    #pragma region ModelData
    bool ModelData::Valid() const {
        return !_meshes.empty();
    }

    void ModelData::Draw(const Camera& camera, const TransformComponent& transform) {
        for (const auto& mesh : _meshes) {
            mesh->Draw();
        }
    }
    #pragma endregion
}