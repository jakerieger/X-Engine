#pragma once

#include "Model.hpp"
#include "ResourceManager.hpp"

namespace x {
    class ModelLoader : public ResourceLoader<ModelData> {
        ModelData LoadImpl(RenderContext& context, const str& path) override {
            ModelData model;

            // Rest of loading implementation

            return model;
        }
    };

    REGISTER_RESOURCE_LOADER(ModelData, ModelLoader)
}