#pragma once

#include "Model.hpp"
#include "RenderContext.hpp"
#include "Common/Types.hpp"

#include <ufbx.h>

namespace x {
    /// @brief Loads mesh data from FBX files or data stored in memory.
    /// @note This is currently outputting faces in the wrong winding order. Use GenericLoader for now.
    class FBXLoader {
        RenderContext& _renderer;

    public:
        explicit FBXLoader(RenderContext& renderer) : _renderer(renderer) {}

        shared_ptr<ModelData> LoadFromFile(const str& path);
        shared_ptr<ModelData> LoadFromMemory(const u8* data, size_t size);

    private:
        unique_ptr<Mesh> ProcessMesh(const ufbx_scene* scene, const ufbx_mesh* mesh);
    };
}