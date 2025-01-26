#include "Model.hpp"

namespace x {
    #pragma region ModelHandle
    void ModelHandle::Draw() {
        if (_modelData) {
            _modelData->Draw();
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

    void ModelData::Draw() {
        for (const auto& mesh : _meshes) {
            mesh->Draw();
        }
    }
    #pragma endregion
}