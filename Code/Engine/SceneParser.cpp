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

    void SceneParser::StateToDescriptor(const SceneState& state, SceneDescriptor& descriptor, const str& sceneName) {
        descriptor.mName = sceneName;

        const auto& camera = state.GetMainCamera();
        CameraDescriptor cameraDescriptor;
        cameraDescriptor.mEye      = camera.GetEye();
        cameraDescriptor.mPosition = camera.GetPosition();
        cameraDescriptor.mFovY     = camera.GetFovY();
        cameraDescriptor.mNearZ    = camera.GetClipPlanes().first;
        cameraDescriptor.mFarZ     = camera.GetClipPlanes().second;
        descriptor.mWorld.mCamera  = cameraDescriptor;

        const auto& sun = state.GetLightState().mSun;
        SunDescriptor sunDescriptor;
        sunDescriptor.mColor           = Float3(sun.mColor.x, sun.mColor.y, sun.mColor.z);
        sunDescriptor.mDirection       = Float3(sun.mDirection.x, sun.mDirection.y, sun.mDirection.z);
        sunDescriptor.mEnabled         = sun.mEnabled;
        sunDescriptor.mIntensity       = sun.mIntensity;
        sunDescriptor.mCastsShadows    = sun.mCastsShadows;
        descriptor.mWorld.mLights.mSun = sunDescriptor;

        const auto& entities = state.GetEntities();
        for (const auto& [id, name] : entities) {
            EntityDescriptor entityDescriptor;
            entityDescriptor.mId   = id.Value();
            entityDescriptor.mName = name;

            const TransformComponent* transform = state.GetComponent<TransformComponent>(id);
            const ModelComponent* model         = state.GetComponent<ModelComponent>(id);
            const BehaviorComponent* behavior   = state.GetComponent<BehaviorComponent>(id);

            if (transform) {
                auto& transformDescriptor     = entityDescriptor.mTransform;
                transformDescriptor.mPosition = transform->GetPosition();
                transformDescriptor.mRotation = transform->GetRotation();
                transformDescriptor.mScale    = transform->GetScale();
            }

            if (model) {
                ModelDescriptor modelDescriptor;
                modelDescriptor.mMeshId         = model->GetModelId();
                modelDescriptor.mMaterialId     = model->GetMaterialId();
                modelDescriptor.mCastsShadows   = model->GetCastsShadows();
                modelDescriptor.mReceiveShadows = model->GetReceiveShadows();
                entityDescriptor.mModel         = modelDescriptor;
            }

            if (behavior) {
                BehaviorDescriptor behaviorDescriptor;
                behaviorDescriptor.mScriptId = behavior->GetScriptId();
                entityDescriptor.mBehavior   = behaviorDescriptor;
            }

            descriptor.mEntities.push_back(entityDescriptor);
        }
    }

    void SceneParser::WriteToFile(const SceneDescriptor& descriptor, const str& filename) {
        YAML::Emitter out;
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
                EmitFloat3(out, camera.mPosition);
                out << YAML::Key << "eye" << YAML::Value << YAML::Flow;
                EmitFloat3(out, camera.mEye);
                out << YAML::Key << "fovY" << YAML::Value << camera.mFovY;
                out << YAML::Key << "nearZ" << YAML::Value << camera.mNearZ;
                out << YAML::Key << "farZ" << YAML::Value << camera.mFarZ;
                out << YAML::EndMap;
                // ====================================================================//

                // ============================== Lights ==============================//
                auto& sun = descriptor.mWorld.mLights.mSun;
                out << YAML::Key << "lights" << YAML::BeginMap;

                // Sun
                out << YAML::Key << "sun" << YAML::BeginMap;
                out << YAML::Key << "enabled" << YAML::Value << sun.mEnabled;
                out << YAML::Key << "intensity" << YAML::Value << sun.mIntensity;
                out << YAML::Key << "color" << YAML::Value << YAML::Flow;
                EmitFloat3(out, sun.mColor);
                out << YAML::Key << "direction" << YAML::Value << YAML::Flow;
                EmitFloat3(out, sun.mDirection);
                out << YAML::Key << "castsShadows" << YAML::Value << sun.mCastsShadows;
                out << YAML::EndMap;

                // Point lights
                auto& pointLights = descriptor.mWorld.mLights.mPointLights;
                out << YAML::Key << "pointLights" << YAML::BeginSeq;
                out << YAML::EndSeq;

                // Area lights
                auto& areaLights = descriptor.mWorld.mLights.mAreaLights;
                out << YAML::Key << "areaLights" << YAML::BeginSeq;
                out << YAML::EndSeq;

                // Spotlights
                auto& spotLights = descriptor.mWorld.mLights.mSpotLights;
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
                    out << YAML::Key << "id" << YAML::Value << entity.mId;
                    out << YAML::Key << "name" << YAML::Value << entity.mName;

                    // Components
                    out << YAML::Key << "components";
                    out << YAML::Value << YAML::BeginMap;

                    // Transform
                    out << YAML::Key << "transform" << YAML::Value << YAML::BeginMap;
                    {
                        auto& transform = entity.mTransform;
                        out << YAML::Key << "position" << YAML::Flow;
                        EmitFloat3(out, transform.mPosition);
                        out << YAML::Key << "rotation" << YAML::Flow;
                        EmitFloat3(out, transform.mRotation);
                        out << YAML::Key << "scale" << YAML::Flow;
                        EmitFloat3(out, transform.mScale);
                    }
                    out << YAML::EndMap;

                    // Model
                    {
                        if (entity.mModel.has_value()) {
                            auto& model = entity.mModel.value();
                            out << YAML::Key << "model" << YAML::Value << YAML::BeginMap;
                            out << YAML::Key << "mesh" << YAML::Value << model.mMeshId;
                            out << YAML::Key << "material" << YAML::Value << model.mMaterialId;
                            out << YAML::Key << "castsShadows" << YAML::Value << model.mCastsShadows;
                            out << YAML::Key << "receiveShadows" << YAML::Value << model.mReceiveShadows;
                            out << YAML::EndMap;
                        }
                    }

                    // Behavior
                    {
                        if (entity.mBehavior.has_value()) {
                            auto& behavior = entity.mBehavior.value();
                            out << YAML::Key << "behavior" << YAML::Value << YAML::BeginMap;
                            out << YAML::Key << "script" << YAML::Value << behavior.mScriptId;
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

        descriptor.mWorld.mCamera.mPosition = ParseFloat3(cameraNode["position"]);
        descriptor.mWorld.mCamera.mEye      = ParseFloat3(cameraNode["eye"]);
        descriptor.mWorld.mCamera.mFovY     = cameraNode["fovY"].as<f32>();
        descriptor.mWorld.mCamera.mNearZ    = cameraNode["nearZ"].as<f32>();
        descriptor.mWorld.mCamera.mFarZ     = cameraNode["farZ"].as<f32>();

        YAML::Node lightNode = world["lights"];
        YAML::Node sunNode   = lightNode["sun"];

        descriptor.mWorld.mLights.mSun.mEnabled      = sunNode["enabled"].as<bool>();
        descriptor.mWorld.mLights.mSun.mIntensity    = sunNode["intensity"].as<f32>();
        descriptor.mWorld.mLights.mSun.mColor        = ParseFloat3(sunNode["color"]);
        descriptor.mWorld.mLights.mSun.mDirection    = ParseFloat3(sunNode["direction"]);
        descriptor.mWorld.mLights.mSun.mCastsShadows = sunNode["castsShadows"].as<bool>();
    }

    void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor) {
        auto& entitiesArray = descriptor.mEntities;

        for (const auto& entity : entities) {
            EntityDescriptor entityDescriptor {};
            entityDescriptor.mId   = entity["id"].as<u64>();
            const auto name        = entity["name"].as<str>();
            entityDescriptor.mName = name;

            YAML::Node componentsNode = entity["components"];

            YAML::Node transformNode = componentsNode["transform"];
            TransformDescriptor transformDescriptor {};
            transformDescriptor.mPosition = ParseFloat3(transformNode["position"]);
            transformDescriptor.mRotation = ParseFloat3(transformNode["rotation"]);
            transformDescriptor.mScale    = ParseFloat3(transformNode["scale"]);

            entityDescriptor.mTransform = transformDescriptor;

            YAML::Node modelNode = componentsNode["model"];
            if (modelNode.IsDefined()) {
                ModelDescriptor modelDescriptor {};
                modelDescriptor.mMeshId         = modelNode["mesh"].as<u64>();
                modelDescriptor.mMaterialId     = modelNode["material"].as<u64>();
                modelDescriptor.mCastsShadows   = modelNode["castsShadows"].as<bool>();
                modelDescriptor.mReceiveShadows = modelNode["receiveShadows"].as<bool>();

                descriptor.mAssetIds.push_back(modelDescriptor.mMeshId);
                descriptor.mAssetIds.push_back(modelDescriptor.mMaterialId);

                entityDescriptor.mModel = modelDescriptor;
            }

            YAML::Node behaviorNode = componentsNode["behavior"];
            if (behaviorNode.IsDefined()) {
                BehaviorDescriptor behaviorDescriptor {};
                behaviorDescriptor.mScriptId = behaviorNode["script"].as<u64>();

                descriptor.mAssetIds.push_back(behaviorDescriptor.mScriptId);

                entityDescriptor.mBehavior = behaviorDescriptor;
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