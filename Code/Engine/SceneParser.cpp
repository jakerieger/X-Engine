#include "SceneParser.hpp"
#include <yaml-cpp/yaml.h>

#include "EngineCommon.hpp"

namespace x {
    static void ParseWorld(const YAML::Node& world, SceneDescriptor& descriptor);
    static void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor);
    static Float3 ParseFloat3(const YAML::Node& node);
    static void EmitFloat3(YAML::Emitter& emitter, const Float3& vector);

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

    void SceneParser::WriteToFile(const SceneDescriptor& descriptor, const str& filename) {
        YAML::Emitter out;

        // TODO: Parse descriptor back to scene descriptor yaml
        out << YAML::BeginMap;
        {
            out << YAML::Key << "version" << YAML::Value << "1.0";
            out << YAML::Key << "name" << YAML::Value << descriptor.mName;
            out << YAML::Key << "description" << YAML::Value << descriptor.mDescription;

            // World
            {
                out << YAML::Key << "world" << YAML::BeginMap;

                // ============================== Camera ==============================//
                auto& camera = descriptor.mWorld.mCamera;
                out << YAML::Key << "camera" << YAML::BeginMap;
                out << YAML::Key << "position" << YAML::Value << YAML::Flow;
                EmitFloat3(out, camera.position);
                out << YAML::Key << "eye" << YAML::Value << YAML::Flow;
                EmitFloat3(out, camera.eye);
                out << YAML::Key << "fovY" << YAML::Value << camera.fovY;
                out << YAML::Key << "nearZ" << YAML::Value << camera.nearZ;
                out << YAML::Key << "farZ" << YAML::Value << camera.farZ;
                out << YAML::EndMap;
                // ====================================================================//

                // ============================== Lights ==============================//
                auto& sun = descriptor.mWorld.mLights.sun;
                out << YAML::Key << "lights" << YAML::BeginMap;

                // Sun
                out << YAML::Key << "sun" << YAML::BeginMap;
                out << YAML::Key << "enabled" << YAML::Value << sun.enabled;
                out << YAML::Key << "intensity" << YAML::Value << sun.intensity;
                out << YAML::Key << "color" << YAML::Value << YAML::Flow;
                EmitFloat3(out, sun.color);
                out << YAML::Key << "direction" << YAML::Value << YAML::Flow;
                EmitFloat3(out, sun.direction);
                out << YAML::Key << "castsShadows" << YAML::Value << sun.castsShadows;
                out << YAML::EndMap;

                // Point lights
                auto& pointLights = descriptor.mWorld.mLights.pointLights;
                out << YAML::Key << "pointLights" << YAML::BeginSeq;
                out << YAML::EndSeq;

                // Area lights
                auto& areaLights = descriptor.mWorld.mLights.areaLights;
                out << YAML::Key << "areaLights" << YAML::BeginSeq;
                out << YAML::EndSeq;

                // Spotlights
                auto& spotLights = descriptor.mWorld.mLights.spotLights;
                out << YAML::Key << "spotLights" << YAML::BeginSeq;
                out << YAML::EndSeq;

                out << YAML::EndMap;
                // ====================================================================//

                out << YAML::EndMap;
            }

            // Entities
            {
                out << YAML::Key << "entities" << YAML::BeginSeq;

                for (auto& entity : descriptor.mEntities) {
                    out << YAML::BeginMap;
                    out << YAML::Key << "id" << YAML::Value << entity.id;
                    out << YAML::Key << "name" << YAML::Value << entity.name;

                    // Components
                    out << YAML::Key << "components";
                    out << YAML::Value << YAML::BeginMap;

                    // Transform
                    out << YAML::Key << "transform" << YAML::Value << YAML::BeginMap;
                    {
                        auto& transform = entity.transform;
                        out << YAML::Key << "position" << YAML::Flow;
                        EmitFloat3(out, transform.position);
                        out << YAML::Key << "rotation" << YAML::Flow;
                        EmitFloat3(out, transform.rotation);
                        out << YAML::Key << "scale" << YAML::Flow;
                        EmitFloat3(out, transform.scale);
                    }
                    out << YAML::EndMap;

                    // Model
                    {
                        if (entity.model.has_value()) {
                            auto& model = entity.model.value();
                            out << YAML::Key << "model" << YAML::Value << YAML::BeginMap;
                            out << YAML::Key << "mesh" << YAML::Value << model.meshId;
                            out << YAML::Key << "material" << YAML::Value << model.materialId;
                            out << YAML::Key << "castsShadows" << YAML::Value << model.castsShadows;
                            out << YAML::Key << "receiveShadows" << YAML::Value << model.receiveShadows;
                            out << YAML::EndMap;
                        }
                    }

                    // Behavior
                    {
                        if (entity.behavior.has_value()) {
                            auto& behavior = entity.behavior.value();
                            out << YAML::Key << "behavior" << YAML::Value << YAML::BeginMap;
                            out << YAML::Key << "script" << YAML::Value << behavior.scriptId;
                            out << YAML::EndMap;
                        }
                    }

                    out << YAML::EndMap;  // components

                    out << YAML::EndMap;  // entity
                }

                out << YAML::EndSeq;
            }
        }
        out << YAML::EndMap;

        FileWriter::WriteAllText(Path(filename), out.c_str());
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
            entityDescriptor.id   = entity["id"].as<u64>();
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

                descriptor.mAssetIds.push_back(modelDescriptor.meshId);
                descriptor.mAssetIds.push_back(modelDescriptor.materialId);

                entityDescriptor.model = modelDescriptor;
            }

            YAML::Node behaviorNode = componentsNode["behavior"];
            if (behaviorNode.IsDefined()) {
                BehaviorDescriptor behaviorDescriptor {};
                behaviorDescriptor.scriptId = behaviorNode["script"].as<u64>();

                descriptor.mAssetIds.push_back(behaviorDescriptor.scriptId);

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

    void EmitFloat3(YAML::Emitter& emitter, const Float3& vector) {
        emitter << YAML::BeginSeq;
        emitter << vector.x << vector.y << vector.z;
        emitter << YAML::EndSeq;
    }
}  // namespace x