#include "Model.hpp"

namespace x {
    #pragma region ModelHandle
    void ModelHandle::Draw() const {
        if (_modelData) {
            _modelData->Draw();
        }
    }

    bool ModelHandle::Valid() const {
        return _modelData && _modelData->Valid();
    }

    size_t ModelHandle::NumMeshes() const {
        return _modelData->_meshes.size();
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