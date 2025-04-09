#include "Scene.hpp"
#include "BehaviorComponent.hpp"
#include "StaticResources.hpp"
#include "SceneParser.hpp"
#include <optional>

#include "WaterMaterial.hpp"

namespace x {
    Scene::Scene(RenderContext& context, ScriptEngine& scriptEngine)
        : mResources(context, X_MEGABYTES(128)), mState(), mInitialState(), mContext(context),
          mScriptEngine(scriptEngine) {}

    Scene::~Scene() {
        Unload();
    }

    void Scene::Load(const SceneDescriptor& descriptor) {
        mState.Reset();
        mInitialState.Reset();
        mOpaqueObjects.clear();
        mTransparentObjects.clear();

        auto& sun = mState.mLights.mSun;

        const auto& sunDescriptor = descriptor.mWorld.mLights.mSun;
        sun.mEnabled              = sunDescriptor.mEnabled;
        sun.mIntensity            = sunDescriptor.mIntensity;
        sun.mColor                = {sunDescriptor.mColor.x, sunDescriptor.mColor.y, sunDescriptor.mColor.z, 1.0f};
        sun.mDirection    = {sunDescriptor.mDirection.x, sunDescriptor.mDirection.y, sunDescriptor.mDirection.z, 0.0f};
        sun.mCastsShadows = sunDescriptor.mCastsShadows;

        for (auto& entity : descriptor.mEntities) {
            const EntityId newEntity = mState.CreateEntity(entity.mName);

            // Create and attach components
            auto transformDescriptor = entity.mTransform;
            auto& transformComponent = mState.AddComponent<TransformComponent>(newEntity);
            transformComponent.SetPosition(transformDescriptor.mPosition);
            transformComponent.SetRotation(transformDescriptor.mRotation);
            transformComponent.SetScale(transformDescriptor.mScale);
            transformComponent.Update();

            if (entity.mModel.has_value()) {
                auto& model          = entity.mModel.value();
                auto& modelComponent = mState.AddComponent<ModelComponent>(newEntity);

                // Load model resource
                if (!mResources.LoadResource<Model>(model.mMeshId)) { X_LOG_FATAL("Failed to load model"); }
                auto modelHandle = mResources.FetchResource<Model>(model.mMeshId);
                if (!modelHandle.Valid()) { X_LOG_FATAL("Failed to fetch model resource"); }

                modelComponent.SetModelHandle(modelHandle)
                  .SetCastsShadows(model.mCastsShadows)
                  .SetReceiveShadows(model.mReceiveShadows)
                  .SetModelId(model.mMeshId)
                  .SetMaterialId(model.mMaterialId);

                // Load material
                const auto materialBytes = AssetManager::GetAssetData(model.mMaterialId);
                if (!materialBytes.has_value()) { X_LOG_FATAL("Failed to load material resource"); }
                MaterialDescriptor matDesc = MaterialParser::Parse(*materialBytes);
                LoadMaterial(matDesc, modelComponent);
            }

            if (entity.mBehavior.has_value()) {
                auto& behavior            = entity.mBehavior.value();
                auto& behaviorComponent   = mState.AddComponent<BehaviorComponent>(newEntity);
                const auto scriptBytecode = AssetManager::GetAssetData(behavior.mScriptId);
                if (!scriptBytecode.has_value()) { X_LOG_FATAL("Failed to load script bytecode") }

                behaviorComponent.Load(behavior.mScriptId);
                if (!mScriptEngine.LoadScript(*scriptBytecode, entity.mName)) { X_LOG_FATAL("Failed to load script"); }
            }

            if (entity.mCamera.has_value()) {
                auto& camera          = entity.mCamera.value();
                auto& cameraComponent = mState.AddComponent<CameraComponent>(newEntity);
                cameraComponent.SetFOVDegrees(camera.mFOV)
                  .SetClipPlanes(camera.mNearZ, camera.mFarZ)
                  .SetOrthographic(camera.mOrthographic)
                  .SetWidthHeight(camera.mWidth, camera.mHeight);
            }
        }

        mInitialState = mState;  // Cache init state so scene can be reset
        mName         = descriptor.mName;
        mLoaded       = true;

        Awake();
        X_LOG_INFO("Loaded scene: '%s'", descriptor.mName.c_str())
    }

    void Scene::Unload() {
        Destroyed();
        mState.Reset();
        mResources.Clear();
        mLoaded = false;
    }

    void Scene::Reset() {
        mState.Reset();
        mResources.Clear();
    }

    void Scene::ResetState() {
        mState.Reset();
        mState = mInitialState;
    }

    void Scene::Awake() {
        for (const auto& [entityId, name] : mState.GetEntities()) {
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

        // Update scene entities
        mTransparentObjects.clear();
        mOpaqueObjects.clear();
        for (const auto& [entityId, name] : mState.GetEntities()) {
            const auto* behaviorComponent = mState.GetComponentMutable<BehaviorComponent>(entityId);
            auto* transformComponent      = mState.GetComponentMutable<TransformComponent>(entityId);
            auto* modelComponent          = mState.GetComponentMutable<ModelComponent>(entityId);
            auto* cameraComponent         = mState.GetComponentMutable<CameraComponent>(entityId);

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
                // TODO: Shitty hack, remove
                auto* waterMaterial = material->As<WaterMaterial>();
                if (waterMaterial) { waterMaterial->SetWaveTime(sceneTime); }
            }

            if (cameraComponent) { cameraComponent->SetPosition(transformComponent->GetPosition()); }
        }

        // Calculate LVP
        // TODO: I only need to update this if either the light direction or the screen size changes; this can be
        // optimized!
        const auto lvp                             = CalculateLightViewProjection(mState.GetLightState().mSun,
                                                      5.0f,  // TODO: this needs tweaking depending on the light height
                                                      mState.GetMainCamera()->GetAspectRatio(),
                                                      mState.GetMainCamera()->GetNearPlane(),
                                                      mState.GetMainCamera()->GetFarPlane());
        mState.GetLightState().mSun.mLightViewProj = XMMatrixTranspose(lvp);

        // Sort transparent objects by distance from camera
        if (mTransparentObjects.size() > 1) {
            const auto cameraPos = mState.GetMainCamera()->GetPosition();
            std::ranges::sort(mTransparentObjects,
                              [cameraPos](const ModelTransformPair& lhs, const ModelTransformPair& rhs) {
                                  const auto& lhsTransform = lhs.second;
                                  const auto& rhsTransform = rhs.second;
                                  const f32 lhsDist        = DistanceSquared(cameraPos, lhsTransform->GetPosition());
                                  const f32 rhsDist        = DistanceSquared(cameraPos, rhsTransform->GetPosition());
                                  return lhsDist > rhsDist;
                              });
        }
    }

    void Scene::Destroyed() {
        for (const auto& [entityId, name] : mState.GetEntities()) {
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
            if (model == nullptr) { continue; }
            Matrix world    = transform->GetTransformMatrix();
            auto view       = mState.GetMainCamera()->GetViewMatrix();
            auto projection = mState.GetMainCamera()->GetProjectionMatrix();
            model->Draw(mContext,
                        {world, view, projection},
                        mState.GetLightState(),
                        mState.GetMainCamera()->GetPosition());
        }
    }

    void Scene::DrawTransparent() {
        for (const auto [model, transform] : mTransparentObjects) {
            if (model == nullptr) { continue; }
            Matrix world    = transform->GetTransformMatrix();
            auto view       = mState.GetMainCamera()->GetViewMatrix();
            auto projection = mState.GetMainCamera()->GetProjectionMatrix();
            model->Draw(mContext,
                        {world, view, projection},
                        mState.GetLightState(),
                        mState.GetMainCamera()->GetPosition());
        }
    }

    SceneState& Scene::GetState() {
        return mState;
    }

    const SceneState& Scene::GetState() const {
        return mState;
    }

    ResourceManager& Scene::GetResourceManager() {
        return mResources;
    }

    const str& Scene::GetName() const {
        return mName;
    }

    bool Scene::Loaded() const {
        return mLoaded;
    }

    void Scene::RegisterVolatiles(vector<Volatile*>& volatiles) {
        volatiles.push_back(mState.GetMainCamera());
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

                ResourceHandle<Texture2D> resource = mResources.FetchResource<Texture2D>(texture.mAssetId);

                if (!resource.Valid()) { X_LOG_FATAL("Failed to fetch texture resource: '%llu'", texture.mAssetId) }

                if (texture.mName == "albedo") { mat->SetAlbedoMap(resource); }
                if (texture.mName == "metallic") { mat->SetMetallicMap(resource); }
                if (texture.mName == "roughness") { mat->SetRoughnessMap(resource); }
                if (texture.mName == "normal") { mat->SetNormalMap(resource); }
            }
        }

        else if (material.mBaseMaterial == "Water") {
            const auto mat = make_shared<WaterMaterial>(mContext, material.mTransparent);
            modelComponent.SetMaterial(mat);

            // Process material properties
        }
    }
}  // namespace x