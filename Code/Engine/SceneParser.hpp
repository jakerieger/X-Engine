#pragma once

#include <span>

#include "Color.hpp"
#include "Math.hpp"
#include "Common/Typedefs.hpp"
#include "SceneState.hpp"

namespace x {
    struct CameraDescriptor {
        f32 mFOV;
        f32 mNearZ;
        f32 mFarZ;
        bool mOrthographic;
        f32 mWidth;
        f32 mHeight;
    };

    struct SunDescriptor {
        bool mEnabled;
        f32 mIntensity;
        Color mColor;
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
        std::optional<CameraDescriptor> mCamera {std::nullopt};
    };

    struct SkyDescriptor {
        Color mSkyColor;
    };

    struct SceneDescriptor {
        str mName;
        str mDescription;

        struct {
            LightDescriptor mLights;
            SkyDescriptor mSky;
        } mWorld;

        vector<EntityDescriptor> mEntities;
        vector<u64> mAssetIds;

        bool IsValid() const {
            return (!mName.empty()) && (mEntities.size() > 0);
        }
    };

    class SceneParser {
    public:
        static bool Parse(const Path& filename, SceneDescriptor& descriptor);
        static bool Parse(std::span<const u8> data, SceneDescriptor& descriptor);
        static bool StateToDescriptor(const SceneState& state, SceneDescriptor& descriptor, const str& sceneName);
        static bool WriteToFile(const SceneDescriptor& descriptor, const Path& filename);
    };
}  // namespace x