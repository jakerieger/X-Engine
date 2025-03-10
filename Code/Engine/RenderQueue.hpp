// Author: Jake Rieger
// Created: 3/9/2025.
//

#pragma once

#include <queue>

#include "ModelComponent.hpp"
#include "Common/Types.hpp"

namespace x {
    struct RenderObject {
        ModelComponent* mModel;
        TransformMatrices mTransforms;
        f32 mDistance;
        bool mTransparent;

        RenderObject(ModelComponent* model, const TransformMatrices& transforms, bool transparent, f32 distance)
            : mModel(model), mTransforms(transforms), mDistance(distance), mTransparent(transparent) {}

        bool operator<(const RenderObject& other) const {
            // Farthest first for transparent objects
            if (mTransparent && other.mTransparent) { return mDistance < other.mDistance; }
            // Opposite for opaque objects
            return !mTransparent && other.mTransparent;
        }
    };

    class RenderQueue {
    public:
        RenderQueue() = default;

        void AddObject(const RenderObject& object) {
            if (object.mTransparent) {
                mTransparentObjects.push(object);
            } else {
                mOpaqueObjects.push_back(object);
            }
        }

        void RenderOpaque(RenderContext& context, const LightState& lights, Float3 eyePosition) const {
            for (auto& object : mOpaqueObjects) {
                object.mModel->Draw(context, object.mTransforms, lights, eyePosition);
            }
        }

        void RenderTransparent(RenderContext& context, const LightState& lights) const {
            for (auto& object : mTransparentObjects) {
                object.mModel->DrawTransparent(context, object.mTransforms, lights);
            }
        }

    private:
        vector<RenderObject> mOpaqueObjects;
        std::priority_queue<RenderObject, vector<RenderObject>, std::greater<RenderObject>> mTransparentObjects;
    };
}  // namespace x
