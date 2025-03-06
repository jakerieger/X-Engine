#pragma once

#include <span>

#include "Math.hpp"
#include "Common/Types.hpp"

namespace x {
    struct CameraDescriptor {
        Float3 position;
        Float3 eye;
        f32 fovY;
        f32 nearZ;
        f32 farZ;
    };

    struct SunDescriptor {
        bool enabled;
        f32 intensity;
        Float3 color;
        Float3 direction;
        bool castsShadows;
    };

    struct PointLightDescriptor {};

    struct AreaLightDescriptor {};

    struct SpotLightDescriptor {};

    struct LightDescriptor {
        SunDescriptor sun;
        vector<PointLightDescriptor> pointLights;
        vector<AreaLightDescriptor> areaLights;
        vector<SpotLightDescriptor> spotLights;
    };

    struct TransformDescriptor {
        Float3 position;
        Float3 rotation;
        Float3 scale;
    };

    struct ModelDescriptor {
        u64 meshId;
        u64 materialId;
        bool castsShadows;
        bool receiveShadows;
    };

    struct BehaviorDescriptor {
        u64 scriptId;
    };

    struct EntityDescriptor {
        str name;
        TransformDescriptor transform;
        std::optional<ModelDescriptor> model       = {};
        std::optional<BehaviorDescriptor> behavior = {};
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
    };

    class SceneParser {
    public:
        static void Parse(const str& filename, SceneDescriptor& descriptor);
        static void Parse(std::span<const u8> data, SceneDescriptor& descriptor);
    };
}  // namespace x