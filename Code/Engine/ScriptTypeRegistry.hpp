#pragma once

#include "ScriptEngine.hpp"
#include "Math.hpp"
#include "TransformComponent.hpp"

namespace x {
    template<>
    struct LuaTypeTraits<Float3> {
        static constexpr std::string_view typeName = "Float3";

        static void RegisterMembers(sol::usertype<Float3>& usertype) {
            usertype["x"] = &Float3::x;
            usertype["y"] = &Float3::y;
            usertype["z"] = &Float3::z;
        }
    };

    template<>
    struct LuaTypeTraits<TransformComponent> {
        static constexpr std::string_view typeName = "Transform";

        static void RegisterMembers(sol::usertype<TransformComponent>& usertype) {
            usertype["GetPosition"] = &TransformComponent::GetPosition;
            usertype["GetRotation"] = &TransformComponent::GetRotation;
            usertype["GetScale"]    = &TransformComponent::GetScale;

            usertype["SetPosition"] = &TransformComponent::SetPosition;
            usertype["SetRotation"] = &TransformComponent::SetRotation;
            usertype["SetScale"]    = &TransformComponent::SetScale;

            usertype["SetRotationX"] = [](TransformComponent& self, const f32 rot) {
                auto currentRot = self.GetRotation();
                currentRot.x    = rot;
                self.SetRotation(currentRot);
            };
            usertype["SetRotationY"] = [](TransformComponent& self, const f32 rot) {
                auto currentRot = self.GetRotation();
                currentRot.y    = rot;
                self.SetRotation(currentRot);
            };
            usertype["SetRotationZ"] = [](TransformComponent& self, const f32 rot) {
                auto currentRot = self.GetRotation();
                currentRot.z    = rot;
                self.SetRotation(currentRot);
            };
        }
    };

    struct BehaviorEntity {
        str name;
        TransformComponent* transform;

        explicit BehaviorEntity(const str& name, TransformComponent* transform) : name(name), transform(transform) {}
    };

    template<>
    struct LuaTypeTraits<BehaviorEntity> {
        static constexpr std::string_view typeName = "Entity";

        static void RegisterMembers(sol::usertype<BehaviorEntity>& usertype) {
            usertype["name"]      = &BehaviorEntity::name;
            usertype["transform"] = &BehaviorEntity::transform;
        }
    };

    template<>
    struct LuaTypeTraits<Camera> {
        static constexpr std::string_view typeName = "Camera";

        static void RegisterMembers(sol::usertype<Camera>& usertype) {
            // TODO: Register camera members
        }
    };
}  // namespace x