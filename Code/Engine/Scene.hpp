#pragma once

#include "Common/Types.hpp"
#include "SceneState.hpp"
#include "TextureLoader.hpp"
#include "ModelLoader.hpp"
#include "ScriptTypeRegistry.hpp"

#include <Vendor/json.hpp>
using json = nlohmann::json;

namespace x {
    namespace scene_schema {
        struct Vec3 {
            f32 x, y, z;

            static Vec3 FromJson(const json& j) {
                return {j["x"].get<f32>(), j["y"].get<f32>(), j["z"].get<f32>()};
            }
        };

        struct Color {
            f32 r, g, b;

            static Color FromJson(const json& j) {
                return {j["r"].get<f32>(), j["g"].get<f32>(), j["b"].get<f32>()};
            }
        };

        struct Camera {
            Vec3 position;
            Vec3 eye;
            f32 fovY;
            f32 nearZ;
            f32 farZ;

            static Camera FromJson(const json& j) {
                return {Vec3::FromJson(j["position"]),
                        Vec3::FromJson(j["eye"]),
                        j["fovY"].get<f32>(),
                        j["nearZ"].get<f32>(),
                        j["farZ"].get<f32>()};
            }
        };

        struct DirectionalLight {
            bool enabled;
            f32 intensity;
            Color color;
            Vec3 direction;
            bool castsShadows;

            static DirectionalLight FromJson(const json& j) {
                return {j["enabled"].get<bool>(),
                        j["intensity"].get<f32>(),
                        Color::FromJson(j["color"]),
                        Vec3::FromJson(j["direction"]),
                        j["castsShadows"].get<bool>()};
            }
        };

        struct Transform {
            Vec3 position;
            Vec3 rotation;
            Vec3 scale;

            static Transform FromJson(const json& j) {
                return {Vec3::FromJson(j["position"]), Vec3::FromJson(j["rotation"]), Vec3::FromJson(j["scale"])};
            }
        };

        struct Model {
            str resource;
            bool castsShadow;
            bool receiveShadow;

            static Model FromJson(const json& j) {
                return {j["resource"].get<str>(), j["castsShadow"].get<bool>(), j["receiveShadow"].get<bool>()};
            }
        };

        struct Behavior {
            str script;

            static Behavior FromJson(const json& j) {
                return {j["script"].get<str>()};
            }
        };
    }

    class Scene {
        ResourceManager _resources;
        SceneState _state;
        RenderContext& _context;
        ScriptEngine& _scriptEngine;
        str _name;
        str _description;
        unordered_map<str, EntityId> _entities;

    public:
        explicit Scene(RenderContext& context, ScriptEngine& scriptEngine);
        ~Scene();

        void Load(const str& path);
        void Unload();

        void Awake();
        void Update(f32 deltaTime);
        void Destroyed();

        SceneState& GetState();
        X_NODISCARD const SceneState& GetState() const;

        void RegisterVolatiles(vector<Volatile*>& volatiles);

    private:
        // void LoadWorld(json& world);
        // void LoadEntities(json& entities);
        // void LoadMaterial(json& material, ModelComponent& modelComponent);
    };
}