#include "Scene.hpp"
#include "BehaviorComponent.hpp"
#include "StaticResources.hpp"
#include "SceneParser.hpp"
#include <optional>

#include "WaterMaterial.hpp"

namespace x {
    Scene::Scene(RenderContext& context, ScriptEngine& scriptEngine)
        : mResources(context, Memory::BYTES_128MB), mState(), mInitialState(), mContext(context),
          mScriptEngine(scriptEngine) {}

    Scene::~Scene() {
        Unload();
    }

    void Scene::Load(const SceneDescriptor& descriptor) {
        mState.Reset();
        mInitialState.Reset();
        mOpaqueObjects.clear();
        mTransparentObjects.clear();

        auto& mainCamera = mState.MainCamera;
        auto& sun        = mState.Lights.Sun;

        const auto& cameraDescriptor = descriptor.mWorld.mCamera;
        mainCamera.SetPosition(Float3ToVectorSet(cameraDescriptor.position));
        mainCamera.SetFOV(cameraDescriptor.fovY);
        mainCamera.SetClipPlanes(cameraDescriptor.nearZ, cameraDescriptor.farZ);

        const auto& sunDescriptor = descriptor.mWorld.mLights.sun;
        sun.enabled               = sunDescriptor.enabled;
        sun.intensity             = sunDescriptor.intensity;
        sun.color                 = {sunDescriptor.color.x, sunDescriptor.color.y, sunDescriptor.color.z, 1.0f};
        sun.direction    = {sunDescriptor.direction.x, sunDescriptor.direction.y, sunDescriptor.direction.z, 0.0f};
        sun.castsShadows = sunDescriptor.castsShadows;

        for (auto& entity : descriptor.mEntities) {
            const EntityId newEntity = mState.CreateEntity();

            // Create and attach components
            auto transformDescriptor = entity.transform;
            auto& transformComponent = mState.AddComponent<TransformComponent>(newEntity);
            transformComponent.SetPosition(transformDescriptor.position);
            transformComponent.SetRotation(transformDescriptor.rotation);
            transformComponent.SetScale(transformDescriptor.scale);
            transformComponent.Update();

            if (entity.model.has_value()) {
                auto& model          = entity.model.value();
                auto& modelComponent = mState.AddComponent<ModelComponent>(newEntity);

                // Load model resource
                if (!mResources.LoadResource<Model>(model.meshId)) { X_LOG_FATAL("Failed to load model"); }
                auto modelHandle = mResources.FetchResource<Model>(model.meshId);
                if (!modelHandle.has_value()) { X_LOG_FATAL("Failed to fetch model resource"); }

                modelComponent.SetModelHandle(*modelHandle)
                  .SetCastsShadows(model.castsShadows)
                  .SetReceiveShadows(model.receiveShadows);

                // Load material
                const auto materialBytes = AssetManager::GetAssetData(model.materialId);
                if (!materialBytes.has_value()) { X_LOG_FATAL("Failed to load material resource"); }
                MaterialDescriptor matDesc = MaterialParser::Parse(*materialBytes);
                LoadMaterial(matDesc, modelComponent);
            }

            if (entity.behavior.has_value()) {
                auto& behavior            = entity.behavior.value();
                auto& behaviorComponent   = mState.AddComponent<BehaviorComponent>(newEntity);
                const auto scriptBytecode = AssetManager::GetAssetData(behavior.scriptId);
                if (!scriptBytecode.has_value()) { X_LOG_FATAL("Failed to load script bytecode") }

                behaviorComponent.Load(behavior.scriptId);
                if (!mScriptEngine.LoadScript(*scriptBytecode, entity.name)) { X_LOG_FATAL("Failed to load script"); }
            }

            mEntities[entity.name] = newEntity;
        }

        mInitialState = mState;  // Cache init state so scene can be reset

        Awake();
        X_LOG_INFO("Loaded scene: '%s'", descriptor.mName.c_str())
    }

    void Scene::Unload() {
        Destroyed();

        mState.Reset();
        mEntities.clear();
        mResources.Clear();
    }

    void Scene::Reset() {
        mState.Reset();
        mEntities.clear();
        mResources.Clear();
    }

    void Scene::ResetToInitialState() {
        mState.Reset();
        mState = mInitialState;
    }

    void Scene::Awake() {
        for (const auto& [name, entityId] : mEntities) {
            const auto* behaviorComponent = mState.GetComponent<BehaviorComponent>(entityId);
            auto* transformComponent      = mState.GetComponentMutable<TransformComponent>(entityId);
            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                mScriptEngine.CallAwakeBehavior(name, entity);
            }
        }
    }

    void Scene::Update(f32 deltaTime) {
        static f32 sceneTime {0.f};
        sceneTime += deltaTime;

        const auto& camera    = mState.GetMainCamera();
        const auto clipPlanes = camera.GetClipPlanes();

        // Calculate LVP
        // TODO: I only need to update this if either the light direction or the screen size changes; this can be
        // optimized!
        const auto lvp                           = CalculateLightViewProjection(mState.GetLightState().Sun,
                                                      5.0f,  // TODO: this needs tweaking depending on the light height
                                                      camera.GetAspectRatio(),
                                                      clipPlanes.first,
                                                      clipPlanes.second);
        mState.GetLightState().Sun.lightViewProj = XMMatrixTranspose(lvp);

        // Update scene entities
        mTransparentObjects.clear();
        mOpaqueObjects.clear();
        for (const auto& [name, entityId] : mEntities) {
            const auto* behaviorComponent = mState.GetComponentMutable<BehaviorComponent>(entityId);
            auto* transformComponent      = mState.GetComponentMutable<TransformComponent>(entityId);
            auto* modelComponent          = mState.GetComponentMutable<ModelComponent>(entityId);

            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                mScriptEngine.CallUpdateBehavior(name, deltaTime, entity);
            }

            // Update transform matrix AFTER the behavior script has run, which may modify the transform
            transformComponent->Update();

            if (modelComponent) {
                const shared_ptr<IMaterial> material = modelComponent->GetMaterial();
                if (material) {
                    if (material->Transparent()) {
                        mTransparentObjects.push_back({modelComponent, transformComponent});
                    } else {
                        mOpaqueObjects.push_back({modelComponent, transformComponent});
                    }
                }
                auto* waterMaterial = material->As<WaterMaterial>();
                if (waterMaterial) { waterMaterial->SetWaveTime(sceneTime); }
            }
        }

        // Sort transparent objects by distance from camera
    }

    void Scene::Destroyed() {
        for (const auto& [name, entityId] : mEntities) {
            const auto* behaviorComponent = mState.GetComponent<BehaviorComponent>(entityId);
            auto* transformComponent      = mState.GetComponentMutable<TransformComponent>(entityId);
            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                mScriptEngine.CallDestroyedBehavior(name, entity);
            }
        }
    }

    void Scene::DrawOpaque() {
        for (const auto [model, transform] : mOpaqueObjects) {
            Matrix world    = transform->GetTransformMatrix();
            auto view       = mState.GetMainCamera().GetViewMatrix();
            auto projection = mState.GetMainCamera().GetProjectionMatrix();
            model->Draw(mContext,
                        {world, view, projection},
                        mState.GetLightState(),
                        mState.GetMainCamera().GetPosition());
        }
    }

    void Scene::DrawTransparent() {
        for (const auto [model, transform] : mTransparentObjects) {
            Matrix world    = transform->GetTransformMatrix();
            auto view       = mState.GetMainCamera().GetViewMatrix();
            auto projection = mState.GetMainCamera().GetProjectionMatrix();
            model->Draw(mContext,
                        {world, view, projection},
                        mState.GetLightState(),
                        mState.GetMainCamera().GetPosition());
        }
    }

    SceneState& Scene::GetState() {
        return mState;
    }

    unordered_map<str, EntityId>& Scene::GetEntities() {
        return mEntities;
    }

    const SceneState& Scene::GetState() const {
        return mState;
    }

    u32 Scene::GetNumEntities() const {
        return mEntities.size();
    }

    ResourceManager& Scene::GetResourceManager() {
        return mResources;
    }

    void Scene::RegisterVolatiles(vector<Volatile*>& volatiles) {
        volatiles.push_back(&mState.GetMainCamera());
    }

    void Scene::LoadMaterial(const MaterialDescriptor& material, ModelComponent& modelComponent) {
        if (material.mBaseMaterial == "PBR") {
            // TODO: Move the base materials to somewhere in static memory so they aren't being created for every single
            // material instance.
            const auto mat = make_shared<PBRMaterial>(mContext, material.mTransparent);
            modelComponent.SetMaterial(mat);

            for (const auto& texture : material.mTextures) {
                // Load texture resource
                if (!mResources.LoadResource<Texture2D>(texture.mAssetId)) {
                    X_LOG_FATAL("Failed to to load texture resource: '%llu'", texture.mAssetId)
                }

                std::optional<ResourceHandle<Texture2D>> resource =
                  mResources.FetchResource<Texture2D>(texture.mAssetId);

                if (!resource.has_value()) { X_LOG_FATAL("Failed to fetch texture resource: '%llu'", texture.mAssetId) }
                if (!resource->Valid()) { X_LOG_FATAL("Resource invalid: '%llu'", texture.mAssetId) }

                if (texture.mName == "albedo") { mat->SetAlbedoMap(*resource); }
                if (texture.mName == "metallic") { mat->SetMetallicMap(*resource); }
                if (texture.mName == "roughness") { mat->SetRoughnessMap(*resource); }
                if (texture.mName == "normal") { mat->SetNormalMap(*resource); }
            }
        }

        else if (material.mBaseMaterial == "Water") {
            const auto mat = make_shared<WaterMaterial>(mContext, material.mTransparent);
            modelComponent.SetMaterial(mat);

            // Process material properties
        }
    }
}  // namespace x