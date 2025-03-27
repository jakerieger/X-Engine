#pragma once

#include <span>

#include "Math.hpp"
#include "Common/Types.hpp"
#include "SceneState.hpp"

namespace x {
    struct CameraDescriptor {
        Float3 mPosition;
        Float3 mLookAt;
        f32 mFovY;
        f32 mNearZ;
        f32 mFarZ;
    };

    struct SunDescriptor {
        bool mEnabled;
        f32 mIntensity;
        Float3 mColor;
        Float3 mDirection;
        bool mCastsShadows;
    };

    struct PointLightDescriptor {};

    struct AreaLightDescriptor {};

    struct SpotLightDescriptor {};

    struct LightDescriptor {
        SunDescriptor mSun;
        vector<PointLightDescriptor> mPointLights;
        vector<AreaLightDescriptor> mAreaLights;
        vector<SpotLightDescriptor> mSpotLights;
    };

    struct TransformDescriptor {
        Float3 mPosition;
        Float3 mRotation;
        Float3 mScale;
    };

    struct ModelDescriptor {
        u64 mMeshId;
        u64 mMaterialId;
        bool mCastsShadows;
        bool mReceiveShadows;
    };

    struct BehaviorDescriptor {
        u64 mScriptId;
    };

    struct EntityDescriptor {
        u64 mId;
        str mName;
        TransformDescriptor mTransform;
        std::optional<ModelDescriptor> mModel {std::nullopt};
        std::optional<BehaviorDescriptor> mBehavior {std::nullopt};
    };

    struct SceneDescriptor {
        str mName;
        str mDescription;

        struct {
            CameraDescriptor mCamera;
            LightDescriptor mLights;
        } mWorld;

        vector<EntityDescriptor> mEntities;
        vector<u64> mAssetIds;

        bool IsValid() const {
            return (!mName.empty()) && (mEntities.size() > 0);
        }
    };

    class SceneParser {
    public:
        static void Parse(const str& filename, SceneDescriptor& descriptor);
        static void Parse(std::span<const u8> data, SceneDescriptor& descriptor);
        static void StateToDescriptor(const SceneState& state, SceneDescriptor& descriptor, const str& sceneName);
        static void WriteToFile(const SceneDescriptor& descriptor, const str& filename);
    };
}  // namespace x