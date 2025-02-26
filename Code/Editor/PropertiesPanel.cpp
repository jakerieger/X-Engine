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

            mTransform.position[0] = pos.x;
            mTransform.position[1] = pos.y;
            mTransform.position[2] = pos.z;

            mTransform.rotation[0] = rot.x;
            mTransform.rotation[1] = rot.y;
            mTransform.rotation[2] = rot.z;

            mTransform.scale[0] = scale.x;
            mTransform.scale[1] = scale.y;
            mTransform.scale[2] = scale.z;
        }
    }

    void PropertiesPanel::Update(EntityId selectedEntity) {
        const auto& game = mEditor.mGame;
        auto& state      = game.GetActiveScene()->GetState();
        UpdateTransformProperties(selectedEntity, state);
    }

    void PropertiesPanel::BehaviorProperties(EntityId selectedEntity, SceneState& state) {
        auto* behaviorComponent = state.GetComponentMutable<BehaviorComponent>(selectedEntity);
        if (behaviorComponent) {
            ImGui::Text("Behavior");
            ImGui::Text("Script: ");
            ImGui::SameLine();
            ImGui::Text("%s", behaviorComponent->GetId().substr(0, 16).c_str());
            ImGui::Spacing();
            if (ImGui::Button("Edit")) {
                auto lang = TextEditor::LanguageDefinition::Lua();
                mTextEditor.SetLanguageDefinition(lang);
                mTextEditor.SetText(behaviorComponent->GetSource());
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
            ImGui::PushFont(mEditor.mFonts["mono"]);
            mTextEditor.Render("LuaEditor",
                               ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 30),
                               true);
            ImGui::PopFont();
            ImGui::PopStyleVar();

            if (ImGui::Button("Save")) {
                auto updatedSource = mTextEditor.GetText();
                if (behaviorComponent && updatedSource != behaviorComponent->GetSource()) {
                    behaviorComponent->UpdateSource(updatedSource);
                    mEditor.mGame.GetScriptEngine().LoadScript(updatedSource, behaviorComponent->GetId());
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

            ImGui::EndPopup();
        }
    }

    void PropertiesPanel::TransformProperties(EntityId selectedEntity, SceneState& state) {
        auto* transformComponent = state.GetComponentMutable<TransformComponent>(selectedEntity);
        if (transformComponent) {
            ImGui::Text("Transform");
            ImGui::DragFloat3("Position", mTransform.position, 0.01f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Rotation", mTransform.rotation, 0.1f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Scale", mTransform.scale, 0.01f, -FLT_MAX, FLT_MAX);

            if (!mEditor.InPlayMode()) {
                transformComponent->SetPosition(
                  {mTransform.position[0], mTransform.position[1], mTransform.position[2]});
                transformComponent->SetRotation(
                  {mTransform.rotation[0], mTransform.rotation[1], mTransform.rotation[2]});
                transformComponent->SetScale({mTransform.scale[0], mTransform.scale[1], mTransform.scale[2]});
                transformComponent->Update();
            }
        }
    }

    void PropertiesPanel::Draw(EntityId selectedEntity) {
        const auto& game = mEditor.mGame;
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