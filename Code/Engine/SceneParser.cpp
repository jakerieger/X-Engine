#include "SceneParser.hpp"
#include "EngineCommon.hpp"
#include "Common/XML.hpp"

namespace x {
    static bool ParseWorld(SceneDescriptor& descriptor, const rapidxml::xml_node<>* worldNode) {
        const auto lightsNode = worldNode->first_node("Lights");
        if (!lightsNode) {
            X_LOG_ERROR("Scene->World->Lights node not found");
            return false;
        }

        const auto sunNode = lightsNode->first_node("Sun");
        if (!sunNode) {
            X_LOG_ERROR("Scene->World->Lights->Sun node not found");
            return false;
        }

        // Sun properties
        const bool sunEnabled     = XML::GetNodeBool(sunNode, "Enabled");
        const f32 sunIntensity    = XML::GetNodeF32(sunNode, "Intensity");
        const Color sunColor      = XML::GetAttrColor(sunNode->first_node("Color"));
        const Float3 sunDirection = XML::GetAttrFloat3(sunNode->first_node("Direction"));
        const bool sunShadows     = XML::GetNodeBool(sunNode, "CastsShadows");

        auto& sunDesc         = descriptor.mWorld.mLights.mSun;
        sunDesc.mEnabled      = sunEnabled;
        sunDesc.mIntensity    = sunIntensity;
        sunDesc.mColor        = sunColor;
        sunDesc.mDirection    = sunDirection;
        sunDesc.mCastsShadows = sunShadows;

        // TODO: PointLights, AreaLights, SpotLights
        {}

        // Sky properties
        const auto skyNode = worldNode->first_node("Sky");
        if (!skyNode) {
            X_LOG_ERROR("Scene->World->Sky node not found");
            return false;
        }

        auto& skyDesc     = descriptor.mWorld.mSky;
        skyDesc.mSkyColor = XML::GetAttrColor(skyNode->first_node("Color"));

        return true;
    }

    static bool ParseEntities(SceneDescriptor& descriptor, const rapidxml::xml_node<>* entitiesNode) {
        auto& entitiesVec = descriptor.mEntities;

        for (const auto* entity = entitiesNode->first_node("Entity"); entity; entity = entity->next_sibling()) {
            EntityDescriptor entityDesc {};

            entityDesc.mId   = std::stoull(entity->first_attribute("id")->value());
            entityDesc.mName = XML::GetAttrStr(entity->first_attribute("name"));

            // Components
            const auto* componentsNode = entity->first_node("Components");
            if (!componentsNode) {
                X_LOG_ERROR("Scene->Entities->Entity->Components node not found");
                return false;
            }

            const auto* transformNode = componentsNode->first_node("Transform");
            if (!transformNode) {
                X_LOG_ERROR("Scene->Entities->Entity->Components->Transform node not found");
                return false;
            }

            // Parse transform node
            const Float3 position = XML::GetAttrFloat3(transformNode->first_node("Position"));
            const Float3 rotation = XML::GetAttrFloat3(transformNode->first_node("Rotation"));
            const Float3 scale    = XML::GetAttrFloat3(transformNode->first_node("Scale"));
            entityDesc.mTransform = TransformDescriptor {
              .mPosition = position,
              .mRotation = rotation,
              .mScale    = scale,
            };

            // Parse model node
            const auto* modelNode = componentsNode->first_node("Model");
            if (modelNode) {
                entityDesc.mModel = ModelDescriptor {
                  .mMeshId         = XML::GetAttrId(modelNode->first_node("Mesh")),
                  .mMaterialId     = XML::GetAttrId(modelNode->first_node("Material")),
                  .mCastsShadows   = XML::GetNodeBool(modelNode, "CastsShadows"),
                  .mReceiveShadows = XML::GetNodeBool(modelNode, "ReceiveShadows"),
                };
            }

            // Parse camera node
            const auto* cameraNode = componentsNode->first_node("Camera");
            if (cameraNode) {
                entityDesc.mCamera = CameraDescriptor {
                  .mFOV          = XML::GetNodeF32(cameraNode, "FOV"),
                  .mNearZ        = XML::GetNodeF32(cameraNode, "NearZ"),
                  .mFarZ         = XML::GetNodeF32(cameraNode, "FarZ"),
                  .mOrthographic = XML::GetNodeBool(cameraNode, "Orthographic"),
                  .mWidth        = XML::GetNodeF32(cameraNode, "Width"),
                  .mHeight       = XML::GetNodeF32(cameraNode, "Height"),
                };
            }

            // Parse behavior node
            const auto* behaviorNode = componentsNode->first_node("Behavior");
            if (behaviorNode) {
                entityDesc.mBehavior = BehaviorDescriptor {
                  .mScriptId = XML::GetAttrId(behaviorNode->first_node("Script")),
                };
            }

            entitiesVec.push_back(entityDesc);
        }

        return true;
    }

    static bool
    ParseDoc(const rapidxml::xml_document<>& doc, SceneDescriptor& descriptor, const char* srcData = nullptr) {
        const auto sceneNode = doc.first_node("Scene");
        if (!sceneNode) {
            X_LOG_ERROR("Scene node not found");
            return false;
        }

        descriptor.mName        = XML::GetAttrStr(sceneNode->first_attribute("name"));
        descriptor.mDescription = XML::GetAttrStr(sceneNode->first_attribute("description"));

        // Parse World
        const auto worldNode = sceneNode->first_node("World");
        if (!worldNode) {
            X_LOG_ERROR("Scene->World node not found");
            return false;
        }
        const bool worldResult = ParseWorld(descriptor, worldNode);

        // Parse Entities
        const auto entitiesNode = sceneNode->first_node("Entities");
        if (!entitiesNode) {
            X_LOG_ERROR("Scene->Entities node not found");
            return false;
        }
        const bool entitiesResult = ParseEntities(descriptor, entitiesNode);

        // Clean up source data if read as bytes
        if (srcData) delete[] srcData;

        return (worldResult && entitiesResult);
    }

    bool SceneParser::Parse(const Path& filename, SceneDescriptor& descriptor) {
        rapidxml::xml_document<> doc;
        if (!XML::ReadFile(filename, doc)) return false;
        return ParseDoc(doc, descriptor);
    }

    bool SceneParser::Parse(std::span<const u8> data, SceneDescriptor& descriptor) {
        rapidxml::xml_document<> doc;

        const auto buffer = new char[data.size() + 1];
        std::copy_n(data.data(), data.size(), buffer);
        buffer[data.size()] = 0;  // null-terminate

        if (!XML::ReadBytes(buffer, sizeof(buffer), doc)) return false;
        return ParseDoc(doc, descriptor, buffer);
    }

    bool SceneParser::WriteToFile(const SceneDescriptor& descriptor, const Path& filename) {
        using namespace rapidxml;
        xml_document<> doc;

        // Root "Scene" node
        xml_node<>* sceneNode = doc.allocate_node(node_element, "Scene");
        sceneNode->append_attribute(doc.allocate_attribute("name", descriptor.mName.c_str()));
        sceneNode->append_attribute(doc.allocate_attribute("description", descriptor.mDescription.c_str()));
        doc.append_node(sceneNode);

        // "World" node
        {
            xml_node<>* worldNode = doc.allocate_node(node_element, "World");
            sceneNode->append_node(worldNode);

            xml_node<>* lightsNode = doc.allocate_node(node_element, "Lights");
            worldNode->append_node(lightsNode);

            xml_node<>* sunNode = doc.allocate_node(node_element, "Sun");
            // Sun
            {
                const auto& sun = descriptor.mWorld.mLights.mSun;

                xml_node<>* sunEnabledNode =
                  doc.allocate_node(node_element, "Enabled", sun.mEnabled ? "true" : "false");
                sunNode->append_node(sunEnabledNode);

                xml_node<>* sunIntensityNode =
                  doc.allocate_node(node_element, "Intensity", X_TOSTR(sun.mIntensity).c_str());
                sunNode->append_node(sunIntensityNode);

                xml_node<>* sunColorNode = XML::MakeColorNode("Color", sun.mColor, doc);
                sunNode->append_node(sunColorNode);

                xml_node<>* sunDirectionNode = XML::MakeFloat3Node("Direction", sun.mDirection, doc);
                sunNode->append_node(sunDirectionNode);

                xml_node<>* sunShadowsNode =
                  doc.allocate_node(node_element, "CastsShadows", sun.mCastsShadows ? "true" : "false");
                sunNode->append_node(sunShadowsNode);
            }
            lightsNode->append_node(sunNode);

            xml_node<>* pointLightsNode = doc.allocate_node(node_element, "PointLights");
            lightsNode->append_node(pointLightsNode);

            xml_node<>* areaLightsNode = doc.allocate_node(node_element, "AreaLights");
            lightsNode->append_node(areaLightsNode);

            xml_node<>* spotLightsNode = doc.allocate_node(node_element, "SpotLights");
            lightsNode->append_node(spotLightsNode);

            xml_node<>* skyNode = doc.allocate_node(node_element, "Sky");
            // Sky
            {
                const auto& sky          = descriptor.mWorld.mSky;
                xml_node<>* skyColorNode = XML::MakeColorNode("Color", sky.mSkyColor, doc);
                skyNode->append_node(skyColorNode);
            }
            worldNode->append_node(skyNode);
        }

        // "Entities" node
        {
            xml_node<>* entitiesNode = doc.allocate_node(node_element, "Entities");

            for (const auto& entity : descriptor.mEntities) {
                xml_node<>* entityNode = doc.allocate_node(node_element, "Entity");
                entityNode->append_attribute(doc.allocate_attribute("id", X_TOSTR(entity.mId).c_str()));
                entityNode->append_attribute(doc.allocate_attribute("name", entity.mName.c_str()));

                xml_node<>* componentsNode = doc.allocate_node(node_element, "Components");

                // Transform
                {
                    auto& transform           = entity.mTransform;
                    xml_node<>* transformNode = doc.allocate_node(node_element, "Transform");

                    xml_node<>* positionNode = XML::MakeFloat3Node("Position", transform.mPosition, doc);
                    transformNode->append_node(positionNode);

                    xml_node<>* rotationNode = XML::MakeFloat3Node("Rotation", transform.mRotation, doc);
                    transformNode->append_node(rotationNode);

                    xml_node<>* scaleNode = XML::MakeFloat3Node("Scale", transform.mScale, doc);
                    transformNode->append_node(scaleNode);

                    componentsNode->append_node(transformNode);
                }

                // Model
                if (entity.mModel.has_value()) {
                    auto& model           = entity.mModel.value();
                    xml_node<>* modelNode = doc.allocate_node(node_element, "Model");

                    xml_node<>* meshNode = doc.allocate_node(node_element, "Mesh");
                    meshNode->append_attribute(doc.allocate_attribute("id", X_TOSTR(model.mMeshId).c_str()));
                    modelNode->append_node(meshNode);

                    xml_node<>* materialNode = doc.allocate_node(node_element, "Material");
                    materialNode->append_attribute(doc.allocate_attribute("id", X_TOSTR(model.mMaterialId).c_str()));
                    modelNode->append_node(materialNode);

                    xml_node<>* castsShadowsNode =
                      doc.allocate_node(node_element, "CastsShadows", model.mCastsShadows ? "true" : "false");
                    modelNode->append_node(castsShadowsNode);

                    xml_node<>* receivesShadowsNode =
                      doc.allocate_node(node_element, "ReceiveShadows", model.mReceiveShadows ? "true" : "false");
                    modelNode->append_node(receivesShadowsNode);

                    componentsNode->append_node(modelNode);
                }

                // Camera
                if (entity.mCamera.has_value()) {
                    auto& camera           = entity.mCamera.value();
                    xml_node<>* cameraNode = doc.allocate_node(node_element, "Camera");

                    xml_node<>* fovNode = doc.allocate_node(node_element, "FOV", X_TOSTR(camera.mFOV).c_str());
                    cameraNode->append_node(fovNode);

                    xml_node<>* nearZNode = doc.allocate_node(node_element, "NearZ", X_TOSTR(camera.mNearZ).c_str());
                    cameraNode->append_node(nearZNode);

                    xml_node<>* farZNode = doc.allocate_node(node_element, "FarZ", X_TOSTR(camera.mFarZ).c_str());
                    cameraNode->append_node(farZNode);

                    xml_node<>* orthographicNode =
                      doc.allocate_node(node_element, "Orthographic", camera.mOrthographic ? "true" : "false");
                    cameraNode->append_node(orthographicNode);

                    xml_node<>* widthNode = doc.allocate_node(node_element, "Width", X_TOSTR(camera.mWidth).c_str());
                    cameraNode->append_node(widthNode);

                    xml_node<>* heightNode = doc.allocate_node(node_element, "Height", X_TOSTR(camera.mHeight).c_str());
                    cameraNode->append_node(heightNode);

                    componentsNode->append_node(cameraNode);
                }

                // Behavior
                if (entity.mBehavior.has_value()) {
                    auto& behavior           = entity.mBehavior.value();
                    xml_node<>* behaviorNode = doc.allocate_node(node_element, "Behavior");

                    xml_node<>* scriptNode = doc.allocate_node(node_element, "Script");
                    scriptNode->append_attribute(doc.allocate_attribute("id", X_TOSTR(behavior.mScriptId).c_str()));
                    behaviorNode->append_node(scriptNode);

                    componentsNode->append_node(behaviorNode);
                }

                entityNode->append_node(componentsNode);
                entitiesNode->append_node(entityNode);
            }

            sceneNode->append_node(entitiesNode);
        }

        return XML::WriteFile(filename, doc);
    }

    bool SceneParser::StateToDescriptor(const SceneState& state, SceneDescriptor& descriptor, const str& sceneName) {
        descriptor.mName = sceneName;

        const auto& sun = state.GetLightState().mSun;
        SunDescriptor sunDescriptor;
        sunDescriptor.mColor           = Color(sun.mColor.x, sun.mColor.y, sun.mColor.z);
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
            const CameraComponent* camera       = state.GetComponent<CameraComponent>(id);

            if (transform) {
                auto& transformDescriptor     = entityDescriptor.mTransform;
                transformDescriptor.mPosition = transform->GetPosition();
                transformDescriptor.mRotation = transform->GetRotation();
                transformDescriptor.mScale    = transform->GetScale();
            }

            if (model) {
                entityDescriptor.mModel = ModelDescriptor {
                  .mMeshId         = model->GetModelId(),
                  .mMaterialId     = model->GetMaterialId(),
                  .mCastsShadows   = model->GetCastsShadows(),
                  .mReceiveShadows = model->GetReceiveShadows(),
                };
            }

            if (behavior) {
                entityDescriptor.mBehavior = BehaviorDescriptor {
                  .mScriptId = behavior->GetScriptId(),
                };
            }

            if (camera) {
                entityDescriptor.mCamera = CameraDescriptor {
                  .mFOV          = camera->GetFOVDegrees(),
                  .mNearZ        = camera->GetNearPlane(),
                  .mFarZ         = camera->GetFarPlane(),
                  .mOrthographic = camera->GetOrthographic(),
                  .mWidth        = camera->GetWidth(),
                  .mHeight       = camera->GetHeight(),
                };
            }

            descriptor.mEntities.push_back(entityDescriptor);
        }

        return true;
    }
}  // namespace x