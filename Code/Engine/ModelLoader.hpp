#pragma once

#include "Model.hpp"
#include "ResourceManager.hpp"

namespace x {
    struct Model {
        vector<f32> vertices;
        vector<u32> indices;
    };

    class ModelLoader : public ResourceLoader<Model> {
        Model LoadImpl(RenderContext& context, const str& path) override {
            Model model;

            // Rest of loading implementation

            return model;
        }
    };

    REGISTER_RESOURCE_LOADER(Model, ModelLoader)
}