// Author: Jake Rieger
// Created: 2/21/2025.
//

#include "PropertiesPanel.hpp"
#include "Engine/Game.hpp"
#include "EditorWindow.hpp"

namespace x::Editor {
    void PropertiesPanel::UpdateTransformProperties(EntityId selectedEntity, SceneState& state) {
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

    void PropertiesPanel::Update(EntityId selectedEntity) {
        const auto& game = _editor._game;
        auto& state      = game.GetActiveScene()->GetState();
        UpdateTransformProperties(selectedEntity, state);
    }

    void PropertiesPanel::BehaviorProperties(EntityId selectedEntity, const SceneState& state) {
        auto* behaviorComponent = state.GetComponent<BehaviorComponent>(selectedEntity);
        if (behaviorComponent) {
            ImGui::Text("Behavior");
            ImGui::Text("Script: ");
            ImGui::SameLine();
            ImGui::Text("%s", behaviorComponent->GetId().substr(0, 16).c_str());
            ImGui::Spacing();
            if (ImGui::Button("Edit")) {
                auto lang = TextEditor::LanguageDefinition::Lua();
                _textEditor.SetLanguageDefinition(lang);
                _textEditor.SetText(behaviorComponent->GetSource());
                ImGui::OpenPopup("Script Editor");
            }
        }

        bool isOpen = true;
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Once);
        if (ImGui::BeginPopupModal("Script Editor",
                                   &isOpen,
                                   ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoSavedSettings)) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save")) {
                        // Handle saving
                    }
                    if (ImGui::MenuItem("Close")) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
            ImGui::PushFont(_editor._fonts["mono"]);
            _textEditor.Render("LuaEditor",
                               ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30),
                               true);
            ImGui::PopFont();
            ImGui::PopStyleVar();

            if (ImGui::Button("Save")) { ImGui::CloseCurrentPopup(); }

            ImGui::SameLine();
            if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

            ImGui::EndPopup();
        }
    }

    void PropertiesPanel::TransformProperties(EntityId selectedEntity, SceneState& state) {
        auto* transformComponent = state.GetComponentMutable<TransformComponent>(selectedEntity);
        if (transformComponent) {
            ImGui::Text("Transform");
            ImGui::DragFloat3("Position", _transform.position, 0.01f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Rotation", _transform.rotation, 0.1f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Scale", _transform.scale, 0.01f, -FLT_MAX, FLT_MAX);

            if (!_editor.InPlayMode()) {
                transformComponent->SetPosition(
                  {_transform.position[0], _transform.position[1], _transform.position[2]});
                transformComponent->SetRotation(
                  {_transform.rotation[0], _transform.rotation[1], _transform.rotation[2]});
                transformComponent->SetScale({_transform.scale[0], _transform.scale[1], _transform.scale[2]});
                transformComponent->Update();
            }
        }
    }

    void PropertiesPanel::Draw(EntityId selectedEntity) {
        const auto& game = _editor._game;
        auto* scene      = game.GetActiveScene();
        auto& resources  = scene->GetResourceManager();
        auto& state      = scene->GetState();

        TransformProperties(selectedEntity, state);
        ImGui::Spacing();

        auto* modelComponent = state.GetComponentMutable<ModelComponent>(selectedEntity);
        if (modelComponent) { ImGui::Text("Model"); }
        ImGui::Spacing();

        BehaviorProperties(selectedEntity, state);
    }
}  // namespace x::Editor