#include "Scene.hpp"
#include "BehaviorComponent.hpp"
#include "StaticResources.hpp"
#include "SceneParser.hpp"
#include <optional>

namespace x {
    Scene::Scene(RenderContext& context, ScriptEngine& scriptEngine)
        : _resources(context, Memory::BYTES_128MB), _state(), _context(context), _scriptEngine(scriptEngine) {}

    Scene::~Scene() {
        Unload();
    }

    void Scene::Load(const str& path) {
        _state.Reset();

        SceneDescriptor descriptor {};
        SceneParser::Parse(path, descriptor);

        auto& mainCamera = _state._mainCamera;
        auto& sun        = _state._lightState.Sun;

        const auto& cameraDescriptor = descriptor.world.camera;
        mainCamera.SetPosition(Float3ToVectorSet(cameraDescriptor.position));
        mainCamera.SetFOV(cameraDescriptor.fovY);
        mainCamera.SetClipPlanes(cameraDescriptor.nearZ, cameraDescriptor.farZ);

        const auto& sunDescriptor = descriptor.world.lights.sun;
        sun.enabled               = sunDescriptor.enabled;
        sun.intensity             = sunDescriptor.intensity;
        sun.color                 = {sunDescriptor.color.x, sunDescriptor.color.y, sunDescriptor.color.z, 1.0f};
        sun.direction    = {sunDescriptor.direction.x, sunDescriptor.direction.y, sunDescriptor.direction.z, 0.0f};
        sun.castsShadows = sunDescriptor.castsShadows;

        for (auto& entity : descriptor.entities) {
            const EntityId newEntity = _state.CreateEntity();

            // Create and attach components
            auto transformDescriptor = entity.transform;
            auto& transformComponent = _state.AddComponent<TransformComponent>(newEntity);
            transformComponent.SetPosition(transformDescriptor.position);
            transformComponent.SetRotation(transformDescriptor.rotation);
            transformComponent.SetScale(transformDescriptor.scale);
            transformComponent.Update();

            if (entity.model.has_value()) {
                auto& model          = entity.model.value();
                auto& modelComponent = _state.AddComponent<ModelComponent>(newEntity);
                modelComponent.SetCastsShadows(model.castsShadows);

                // Load model resource
                if (!_resources.LoadResource<Model>(model.resource)) { X_LOG_FATAL("Failed to load model"); }
                auto modelHandle = _resources.FetchResource<Model>(model.resource);
                if (!modelHandle.has_value()) { X_LOG_FATAL("Failed to fetch model resource"); }
                modelComponent.SetModelHandle(*modelHandle);

                // Load material
                MaterialDescriptor matDesc = MaterialParser::Parse(model.material);
                LoadMaterial(matDesc, modelComponent);
            }

            if (entity.behavior.has_value()) {
                auto& behavior          = entity.behavior.value();
                auto& behaviorComponent = _state.AddComponent<BehaviorComponent>(newEntity);
                behaviorComponent.LoadFromFile(behavior.script);

                const auto loadResult =
                  _scriptEngine.LoadScript(behaviorComponent.GetSource(), behaviorComponent.GetId());
                if (!loadResult) { X_LOG_FATAL("Failed to load behavior script"); }
            }

            _entities[entity.name] = newEntity;
        }

        Awake();
        X_LOG_INFO("Loaded scene: '%s'", path.c_str())
    }

    void Scene::Unload() {
        Destroyed();

        _state.Reset();
        _entities.clear();
        _resources.Clear();
    }

    void Scene::Awake() {
        for (const auto& [name, entityId] : _entities) {
            const auto* behaviorComponent = _state.GetComponent<BehaviorComponent>(entityId);
            auto* transformComponent      = _state.GetComponentMutable<TransformComponent>(entityId);
            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                _scriptEngine.CallAwakeBehavior(behaviorComponent->GetId(), entity);
            }
        }
    }

    void Scene::Update(f32 deltaTime) {
        const auto& camera    = _state.GetMainCamera();
        const auto clipPlanes = camera.GetClipPlanes();

        // Calculate LVP
        // TODO: I only need to update this if either the light direction or the screen size changes; this can be
        // optimized!
        const auto lvp                           = CalculateLightViewProjection(_state.GetLightState().Sun,
                                                      10.0f,
                                                      camera.GetAspectRatio(),
                                                      clipPlanes.first,
                                                      clipPlanes.second);
        _state.GetLightState().Sun.lightViewProj = XMMatrixTranspose(lvp);

        // Update scene entities
        for (const auto& [name, entityId] : _entities) {
            const auto* behaviorComponent = _state.GetComponentMutable<BehaviorComponent>(entityId);
            auto* transformComponent      = _state.GetComponentMutable<TransformComponent>(entityId);
            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                _scriptEngine.CallUpdateBehavior(behaviorComponent->GetId(), deltaTime, entity);
            }

            if (transformComponent) {
                // Instead of re-calculating matrices every frame, the Transform component uses lazy updating.
                // This does nothing if no transform values have changed between frames.
                transformComponent->Update();
            }
        }
    }

    void Scene::Destroyed() {
        for (const auto& [name, entityId] : _entities) {
            const auto* behaviorComponent = _state.GetComponent<BehaviorComponent>(entityId);
            auto* transformComponent      = _state.GetComponentMutable<TransformComponent>(entityId);
            if (behaviorComponent) {
                BehaviorEntity entity(name, transformComponent);
                _scriptEngine.CallDestroyedBehavior(behaviorComponent->GetId(), entity);
            }
        }
    }

    SceneState& Scene::GetState() {
        return _state;
    }

    const SceneState& Scene::GetState() const {
        return _state;
    }

    void Scene::RegisterVolatiles(vector<Volatile*>& volatiles) {
        volatiles.push_back(&_state.GetMainCamera());
    }

    void Scene::LoadMaterial(const MaterialDescriptor& material, ModelComponent& modelComponent) {
        if (material.baseMaterial == "PBR") {
            // TODO: Move the base materials to somewhere in static memory so they aren't being created for every single
            // material instance.
            const auto baseMatHandle = PBRMaterial::Create(_context);
            modelComponent.SetMaterial(baseMatHandle);
            PBRMaterialInstance& instance = modelComponent.GetMaterialInstance();

            for (const auto& texture : material.textures) {
                // Load texture resource
                if (!_resources.LoadResource<Texture2D>(texture.resource)) {
                    X_LOG_FATAL("Failed to to load texture resource: '%s'", texture.resource)
                }

                std::optional<ResourceHandle<Texture2D>> resource =
                  _resources.FetchResource<Texture2D>(texture.resource);

                if (!resource.has_value()) { X_LOG_FATAL("Failed to fetch texture resource: '%s'", texture.resource) }
                if (!resource->Valid()) { X_LOG_FATAL("Resource invalid: '%s'", texture.resource) }

                if (texture.name == "albedo") { instance.SetAlbedoMap(*resource); }
                if (texture.name == "metallic") { instance.SetMetallicMap(*resource); }
                if (texture.name == "roughness") { instance.SetRoughnessMap(*resource); }
                if (texture.name == "normal") { instance.SetNormalMap(*resource); }
            }
        }
    }
}  // namespace x