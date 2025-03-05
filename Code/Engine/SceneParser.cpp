#include "SceneParser.hpp"
#include <yaml-cpp/yaml.h>

#include "EngineCommon.hpp"

namespace x {
    static void ParseWorld(const YAML::Node& world, SceneDescriptor& descriptor);
    static void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor);
    static Float3 ParseFloat3(const YAML::Node& node);

    static void ParseFromNode(const YAML::Node& scene, SceneDescriptor& descriptor) {
        const auto name = scene["name"].as<str>();
        const auto desc = scene["description"].as<str>();

        descriptor.mName        = name;
        descriptor.mDescription = desc;

        ParseWorld(scene["world"], descriptor);

        if (const auto entities = scene["entities"]; entities.IsDefined() && entities.size() > 0) {
            ParseEntities(entities, descriptor);
        }
    }

    void SceneParser::Parse(const str& filename, SceneDescriptor& descriptor) {
        const YAML::Node scene = YAML::LoadFile(filename);
        ParseFromNode(scene, descriptor);
    }

    void SceneParser::Parse(std::span<const u8> data, SceneDescriptor& descriptor) {
        const auto sceneContent = RCAST<const char*>(data.data());
        const YAML::Node scene  = YAML::Load(sceneContent);
        ParseFromNode(scene, descriptor);
    }

    void ParseWorld(const YAML::Node& world, SceneDescriptor& descriptor) {
        YAML::Node cameraNode = world["camera"];

        descriptor.mWorld.mCamera.position = ParseFloat3(cameraNode["position"]);
        descriptor.mWorld.mCamera.eye      = ParseFloat3(cameraNode["eye"]);
        descriptor.mWorld.mCamera.fovY     = cameraNode["fovY"].as<f32>();
        descriptor.mWorld.mCamera.nearZ    = cameraNode["nearZ"].as<f32>();
        descriptor.mWorld.mCamera.farZ     = cameraNode["farZ"].as<f32>();

        YAML::Node lightNode = world["lights"];
        YAML::Node sunNode   = lightNode["sun"];

        descriptor.mWorld.mLights.sun.enabled      = sunNode["enabled"].as<bool>();
        descriptor.mWorld.mLights.sun.intensity    = sunNode["intensity"].as<f32>();
        descriptor.mWorld.mLights.sun.color        = ParseFloat3(sunNode["color"]);
        descriptor.mWorld.mLights.sun.direction    = ParseFloat3(sunNode["direction"]);
        descriptor.mWorld.mLights.sun.castsShadows = sunNode["castsShadows"].as<bool>();
    }

    void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor) {
        auto& entitiesArray = descriptor.mEntities;

        for (const auto& entity : entities) {
            EntityDescriptor entityDescriptor {};
            const auto name       = entity["name"].as<str>();
            entityDescriptor.name = name;

            YAML::Node componentsNode = entity["components"];

            YAML::Node transformNode = componentsNode["transform"];
            TransformDescriptor transformDescriptor {};
            transformDescriptor.position = ParseFloat3(transformNode["position"]);
            transformDescriptor.rotation = ParseFloat3(transformNode["rotation"]);
            transformDescriptor.scale    = ParseFloat3(transformNode["scale"]);

            entityDescriptor.transform = transformDescriptor;

            YAML::Node modelNode = componentsNode["model"];
            if (modelNode.IsDefined()) {
                ModelDescriptor modelDescriptor {};
                modelDescriptor.meshId         = modelNode["mesh"].as<u64>();
                modelDescriptor.materialId     = modelNode["material"].as<u64>();
                modelDescriptor.castsShadows   = modelNode["castsShadows"].as<bool>();
                modelDescriptor.receiveShadows = modelNode["receiveShadows"].as<bool>();

                entityDescriptor.model = modelDescriptor;
            }

            YAML::Node behaviorNode = componentsNode["behavior"];
            if (behaviorNode.IsDefined()) {
                BehaviorDescriptor behaviorDescriptor {};
                behaviorDescriptor.script = behaviorNode["script"].as<str>();

                entityDescriptor.behavior = behaviorDescriptor;
            }

            entitiesArray.push_back(entityDescriptor);
        }
    }

    Float3 ParseFloat3(const YAML::Node& node) {
        if (node.IsSequence() && node.size() == 3) {
            return {node[0].as<f32>(), node[1].as<f32>(), node[2].as<f32>()};
        } else {
            X_LOG_ERROR("Failed to parse float3 in scene descriptor.")
            return {std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max()};
        }
    }
}  // namespace x