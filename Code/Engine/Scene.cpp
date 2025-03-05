#include "Scene.hpp"
#include "BehaviorComponent.hpp"
#include "StaticResources.hpp"
#include "SceneParser.hpp"
#include <optional>

namespace x {
    Scene::Scene(RenderContext& context, ScriptEngine& scriptEngine)
        : mResources(context, Memory::BYTES_128MB), mState(), mInitialState(), mContext(context),
          mScriptEngine(scriptEngine) {}

    Scene::~Scene() {
        Unload();
    }

    void Scene::Load(const str& path) {
        mState.Reset();
        mInitialState.Reset();

        SceneDescriptor descriptor {};
        SceneParser::Parse(path, descriptor);

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
                modelComponent.SetCastsShadows(model.castsShadows);

                // Load model resource
                if (!mResources.LoadResource<Model>(model.assetId)) { X_LOG_FATAL("Failed to load model"); }
                auto modelHandle = mResources.FetchResource<Model>(model.assetId);
                if (!modelHandle.has_value()) { X_LOG_FATAL("Failed to fetch model resource"); }
                modelComponent.SetModelHandle(*modelHandle);

                // Load material
                MaterialDescriptor matDesc = MaterialParser::Parse(model.material);
                LoadMaterial(matDesc, modelComponent);
            }

            if (entity.behavior.has_value()) {
                auto& behavior          = entity.behavior.value();
                auto& behaviorComponent = mState.AddComponent<BehaviorComponent>(newEntity);
                behaviorComponent.LoadFromFile(behavior.script);

                const auto loadResult =
                  mScriptEngine.LoadScript(behaviorComponent.GetSource(), behaviorComponent.GetId());
                if (!loadResult) { X_LOG_FATAL("Failed to load behavior script"); }
            }

            mEntities[entity.name] = newEntity;
        }

        mInitialState = mState;  // Cache init state so scene can be reset

        Awake();
        X_LOG_INFO("Loaded scene: '%s'", path.c_str())
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
                mScriptEngine.CallAwakeBehavior(behaviorComponent->GetId(), entity);
            }
        }
    }

    void Scene::Update(f32 deltaTime) {
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
        for (const auto& [name, entityId] : mEntities) {
            const auto* behaviorComponent = mState.GetComponentMutable<BehaviorComponent>(entityId);
            auto* transformComponent      = mState.GetComponentMutable<TransformComponent>(entityId);
            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                mScriptEngine.CallUpdateBehavior(behaviorComponent->GetId(), deltaTime, entity);
            }

            if (transformComponent) {
                // Instead of re-calculating matrices every frame, the Transform component uses lazy updating.
                // This does nothing if no transform values have changed between frames.
                transformComponent->Update();
            }
        }
    }

    void Scene::Destroyed() {
        for (const auto& [name, entityId] : mEntities) {
            const auto* behaviorComponent = mState.GetComponent<BehaviorComponent>(entityId);
            auto* transformComponent      = mState.GetComponentMutable<TransformComponent>(entityId);
            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                mScriptEngine.CallDestroyedBehavior(behaviorComponent->GetId(), entity);
            }
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
        if (material.baseMaterial == "PBR") {
            // TODO: Move the base materials to somewhere in static memory so they aren't being created for every single
            // material instance.
            const auto mat = make_shared<PBRMaterial>(mContext);
            modelComponent.SetMaterial(mat);

            for (const auto& texture : material.textures) {
                // Load texture resource
                if (!mResources.LoadResource<Texture2D>(texture.assetId)) {
                    X_LOG_FATAL("Failed to to load texture resource: '%llu'", texture.assetId)
                }

                std::optional<ResourceHandle<Texture2D>> resource =
                  mResources.FetchResource<Texture2D>(texture.assetId);

                if (!resource.has_value()) { X_LOG_FATAL("Failed to fetch texture resource: '%llu'", texture.assetId) }
                if (!resource->Valid()) { X_LOG_FATAL("Resource invalid: '%llu'", texture.assetId) }

                if (texture.name == "albedo") { mat->SetAlbedoMap(*resource); }
                if (texture.name == "metallic") { mat->SetMetallicMap(*resource); }
                if (texture.name == "roughness") { mat->SetRoughnessMap(*resource); }
                if (texture.name == "normal") { mat->SetNormalMap(*resource); }
            }
        }
    }
}  // namespace x