#pragma once

#include "Model.hpp"
#include "Renderer.hpp"
#include "Common/Types.hpp"

#include <ufbx.h>

namespace x {
    /// @brief Loads mesh data from FBX files or data stored in memory. Prefer this over GenericLoader when possible.
    class FBXLoader {
        Renderer& _renderer;

    public:
        explicit FBXLoader(Renderer& renderer) : _renderer(renderer) {}

        shared_ptr<ModelData> LoadFromFile(const str& path);
        shared_ptr<ModelData> LoadFromMemory(const u8* data, size_t size);

    private:
        unique_ptr<Mesh> ProcessMesh(const ufbx_scene* scene, const ufbx_mesh* mesh);
    };
}