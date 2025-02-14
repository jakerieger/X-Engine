#include "Scene.hpp"
#include "BehaviorComponent.hpp"
#include "StaticResources.hpp"
#include <ranges>

namespace x {
    Scene::Scene(RenderContext& context, ScriptEngine& scriptEngine): _resources(context, Memory::BYTES_1GB), _state(),
                                                                      _context(context),
                                                                      _scriptEngine(scriptEngine) {}

    Scene::~Scene() {
        Unload();
    }

    void Scene::Load(const str& path) {
        _state.Reset();

        std::ifstream f(path);
        json sceneJson = json::parse(f);

        if (sceneJson["version"].get<str>() != "1.0") {
            X_PANIC("Scene schema wrong version");
        }

        _name        = sceneJson["name"].get<str>();
        _description = sceneJson["description"].get<str>();

        auto worldJson = sceneJson["world"];
        LoadWorld(worldJson);

        auto entitiesJson = sceneJson["entities"];
        LoadEntities(entitiesJson);

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
        // TODO: I only need to update this if either the light direction or the screen size changes; this can be optimized!
        const auto lvp =
            CalculateLightViewProjection(_state.GetLightState().Sun,
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

    void Scene::LoadWorld(json& world) {
        auto cameraJson             = world["camera"];
        scene_schema::Camera camera = scene_schema::Camera::FromJson(cameraJson);

        // Update camera
        _state._mainCamera.SetPosition(Vector{camera.position.x, camera.position.y, camera.position.z});
        // TODO: Add setter for eye
        _state._mainCamera.SetFOV(camera.fovY);
        _state._mainCamera.SetClipPlanes(camera.nearZ, camera.farZ);

        auto sunJson                       = world["lights"]["sun"];
        scene_schema::DirectionalLight sun = scene_schema::DirectionalLight::FromJson(sunJson);

        // Update sun
        auto& sunState       = _state._lightState.Sun;
        sunState.enabled     = sun.enabled;
        sunState.intensity   = sun.intensity;
        sunState.color       = Float4{sun.color.r, sun.color.g, sun.color.b, 1.0f};
        sunState.direction   = Float4{sun.direction.x, sun.direction.y, sun.direction.z, 0.0f};
        sunState.castsShadow = sun.castsShadows;
    }

    void Scene::LoadEntities(json& entities) {
        for (auto& entity : entities) {
            const auto entityId    = _state.CreateEntity();
            const auto& components = entity["components"];

            if (components.contains("transform")) {
                scene_schema::Transform transform = scene_schema::Transform::FromJson(components["transform"]);
                auto& transformComponent          = _state.AddComponent<TransformComponent>(entityId);

                transformComponent.SetPosition({transform.position.x, transform.position.y, transform.position.z});
                transformComponent.SetRotation({transform.rotation.x, transform.rotation.y, transform.rotation.z});
                transformComponent.SetScale({transform.scale.x, transform.scale.y, transform.scale.z});
                transformComponent.Update();
            }

            if (components.contains("model")) {
                scene_schema::Model model = scene_schema::Model::FromJson(components["model"]);

                // Load resource
                if (!_resources.LoadResource<Model>(model.resource)) {
                    X_LOG_FATAL("Failed to load model resource when loading scene '%s'", model.resource.c_str())
                }

                auto modelHandle = _resources.FetchResource<Model>(model.resource);
                if (!modelHandle.has_value()) {
                    X_LOG_FATAL("Failed to fetch model resource: %s", model.resource.c_str());
                }

                auto& modelComponent = _state.AddComponent<ModelComponent>(entityId);
                modelComponent.SetModelHandle(*modelHandle);

                if (components["model"].contains("material")) {
                    const auto& mat     = components["model"]["material"];
                    const auto& matPath = mat.get<str>();
                    if (!matPath.empty()) {
                        std::ifstream f(matPath);
                        json matJson = json::parse(f);
                        LoadMaterial(matJson, modelComponent);
                    }
                }

                modelComponent.SetCastsShadows(model.castsShadow);
            }

            if (components.contains("behavior")) {
                scene_schema::Behavior behavior = scene_schema::Behavior::FromJson(components["behavior"]);
                auto& behaviorComponent         = _state.AddComponent<BehaviorComponent>(entityId);
                behaviorComponent.LoadFromFile(behavior.script);

                auto loadResult = _scriptEngine.LoadScript(behaviorComponent.GetSource(),
                                                           behaviorComponent.GetId());
                if (!loadResult) {
                    X_LOG_FATAL("ScriptEngine failed to load script '%s' for entity '%s'",
                                behavior.script.c_str(),
                                entities["name"].get<str>().c_str());
                }
            }

            _entities[entity["name"].get<str>()] = entityId;
        }
    }

    void Scene::LoadMaterial(json& material, ModelComponent& modelComponent) {
        const auto& baseMaterial = material["baseMaterial"].get<str>();
        if (baseMaterial == "PBR") {
            // TODO: Move the PBR Material handle to somewhere in static memory so we aren't creating a new one for every material
            // At the moment, this kind of defeats the purpose of having base materials and material instances.
            const auto baseMatHandle = PBRMaterial::Create(_context);
            modelComponent.SetMaterial(baseMatHandle);
            auto& matInstance = modelComponent.GetMaterialInstance();

            // Read material properties and load textures
            const auto& texturesJson = material["textures"];
            for (auto& texJson : texturesJson) {
                const auto& name     = texJson["name"].get<str>();
                const auto& resource = texJson["resource"].get<str>();

                const bool loaded = _resources.LoadResource<Texture2D>(resource);
                if (!loaded) { X_LOG_FATAL("Failed to load texture '%s' for '%s'.", resource.c_str(), name.c_str()); }

                // TODO: I feel like I don't need to return an optional since ResourceHandle has an internal method for checking data validity
                // Might refactor this
                std::optional<ResourceHandle<Texture2D>> fetchedResource = _resources.FetchResource<
                    Texture2D>(resource);

                if (!fetchedResource.has_value())
                    X_LOG_FATAL("Failed to fetch resource '%s'", resource.c_str())

                if (!fetchedResource->Valid())
                    X_LOG_FATAL("Resource '%s' is not valid", resource.c_str())

                if (name == "albedo") {
                    // extracting value from std::optional, not de-referencing ptr!
                    matInstance.SetAlbedoMap(*fetchedResource);
                    continue;
                }
                if (name == "metallic") {
                    matInstance.SetMetallicMap(*fetchedResource);
                    continue;
                }
                if (name == "roughness") {
                    matInstance.SetRoughnessMap(*fetchedResource);
                    continue;
                }
                if (name == "normal") {
                    matInstance.SetNormalMap(*fetchedResource);
                }
            }
        }
    }
}