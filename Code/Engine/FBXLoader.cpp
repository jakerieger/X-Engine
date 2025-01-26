#include "FBXLoader.hpp"

namespace x {
    shared_ptr<ModelData> FBXLoader::LoadFromFile(const str& path) {
        const ufbx_load_opts opts = {
            .target_axes = ufbx_axes_left_handed_z_up,
            .target_unit_meters = 1.0f,
        };

        ufbx_error err;

        ufbx_scene* scene = ufbx_load_file(path.c_str(), &opts, &err);
        if (!scene) {
            _i_ = fprintf(stderr, "UFBX error loading scene: %s\n", err.info);
            return None;
        }

        auto modelData = make_shared<ModelData>(_renderer);

        for (auto i = 0; i < scene->meshes.count; ++i) {
            const auto* mesh = scene->meshes[i];
            modelData->_meshes.push_back(ProcessMesh(scene, mesh));
        }
        ufbx_free_scene(scene);

        return modelData;
    }

    shared_ptr<ModelData> FBXLoader::LoadFromMemory(const u8* data, const size_t size) {
        const ufbx_load_opts opts = {
            .target_axes = ufbx_axes_left_handed_z_up,
            .target_unit_meters = 1.0f,
        };

        ufbx_error err;

        ufbx_scene* scene = ufbx_load_memory(data, size, &opts, &err);
        if (!scene) {
            _i_ = fprintf(stderr, "UFBX error loading scene: %s\n", err.info);
            return None;
        }

        auto modelData = make_shared<ModelData>(_renderer);

        for (auto i = 0; i < scene->meshes.count; ++i) {
            const auto* mesh = scene->meshes[i];
            modelData->_meshes.push_back(ProcessMesh(scene, mesh));
        }
        ufbx_free_scene(scene);

        return modelData;
    }

    unique_ptr<Mesh> FBXLoader::ProcessMesh(const ufbx_scene* scene, const ufbx_mesh* mesh) {
        vector<VSInputPBR> vertices;
        vector<u32> indices;

        for (auto i = 0; i < mesh->num_vertices; i++) {
            const auto pos    = mesh->vertices[i];
            const auto normal = mesh->vertex_normal[i];
            const auto uv     = mesh->vertex_uv[i];

            VSInputPBR vertex;

            vertex.position.x = CAST<f32>(pos.x);
            vertex.position.y = CAST<f32>(pos.y);
            vertex.position.z = CAST<f32>(pos.z);

            vertex.normal.x = CAST<f32>(normal.x);
            vertex.normal.y = CAST<f32>(normal.y);
            vertex.normal.z = CAST<f32>(normal.z);

            vertex.texCoord.x = CAST<f32>(uv.x);
            vertex.texCoord.y = CAST<f32>(uv.y);

            vertices.push_back(vertex);
        }

        for (auto i = 0; i < mesh->num_indices; i++) {
            indices.push_back(mesh->vertex_indices[i]);
        }

        return make_unique<Mesh>(_renderer, vertices, indices);
    }
}