#include "SceneParser.hpp"
#include <yaml-cpp/yaml.h>

#include "EngineCommon.hpp"

namespace x {
    static void ParseWorld(const YAML::Node& world, SceneDescriptor& descriptor);
    static void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor);
    static Float3 ParseFloat3(const YAML::Node& node);

    void SceneParser::Parse(const str& filename, SceneDescriptor& descriptor) {
        YAML::Node scene = YAML::LoadFile(filename);

        const auto name = scene["name"].as<str>();
        const auto desc = scene["description"].as<str>();

        descriptor.name        = name;
        descriptor.description = desc;

        ParseWorld(scene["world"], descriptor);

        if (const auto entities = scene["entities"]; entities.IsDefined() && entities.size() > 0) {
            ParseEntities(entities, descriptor);
        }
    }

    void ParseWorld(const YAML::Node& world, SceneDescriptor& descriptor) {
        YAML::Node cameraNode = world["camera"];

        descriptor.world.camera.position = ParseFloat3(cameraNode["position"]);
        descriptor.world.camera.eye      = ParseFloat3(cameraNode["eye"]);
        descriptor.world.camera.fovY     = cameraNode["fovY"].as<f32>();
        descriptor.world.camera.nearZ    = cameraNode["nearZ"].as<f32>();
        descriptor.world.camera.farZ     = cameraNode["farZ"].as<f32>();

        YAML::Node lightNode = world["lights"];
        YAML::Node sunNode   = lightNode["sun"];

        descriptor.world.lights.sun.enabled      = sunNode["enabled"].as<bool>();
        descriptor.world.lights.sun.intensity    = sunNode["intensity"].as<f32>();
        descriptor.world.lights.sun.color        = ParseFloat3(sunNode["color"]);
        descriptor.world.lights.sun.direction    = ParseFloat3(sunNode["direction"]);
        descriptor.world.lights.sun.castsShadows = sunNode["castsShadows"].as<bool>();
    }

    void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor) {
        auto& entitiesArray = descriptor.entities;

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
                modelDescriptor.resource       = modelNode["resource"].as<str>();
                modelDescriptor.material       = modelNode["material"].as<str>();
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