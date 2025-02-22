// Author: Jake Rieger
// Created: 2/21/2025.
//

#include "PropertiesPanel.hpp"
#include "Engine/Game.hpp"

namespace x::Editor {
    void PropertiesPanel::Update(EntityId selectedEntity) {
        auto& state              = _game.GetActiveScene()->GetState();
        auto* transformComponent = state.GetComponent<TransformComponent>(selectedEntity);
        if (transformComponent) {
            auto pos   = transformComponent->GetPosition();
            auto rot   = transformComponent->GetRotation();
            auto scale = transformComponent->GetScale();

            _transform.position[0] = pos.x;
            _transform.position[1] = pos.y;
            _transform.position[2] = pos.z;

            _transform.rotation[0] = rot.x;
            _transform.rotation[1] = rot.y;
            _transform.rotation[2] = rot.z;

            _transform.scale[0] = scale.x;
            _transform.scale[1] = scale.y;
            _transform.scale[2] = scale.z;
        }
    }

    void PropertiesPanel::Draw(EntityId selectedEntity) {
        // Push changes to game state
        auto& state              = _game.GetActiveScene()->GetState();
        auto* transformComponent = state.GetComponentMutable<TransformComponent>(selectedEntity);
        if (transformComponent) {
            ImGui::DragFloat3("Position", _transform.position, 0.01f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Rotation", _transform.rotation, 0.01f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Scale", _transform.scale, 0.01f, -FLT_MAX, FLT_MAX);

            transformComponent->SetPosition({_transform.position[0], _transform.position[1], _transform.position[2]});
            //
            transformComponent->Update();
        }
    }
}  // namespace x::Editor