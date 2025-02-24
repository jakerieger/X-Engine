// Author: Jake Rieger
// Created: 2/21/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/EntityId.hpp"
#include "Engine/SceneState.hpp"

#include <TextEditor.h>

namespace x::Editor {
    class EditorWindow;

    struct TransformValues {
        f32 position[3];
        f32 rotation[3];
        f32 scale[3];
    };

    class PropertiesPanel {
        EditorWindow& _editor;

    public:
        explicit PropertiesPanel(EditorWindow& editor) : _editor(editor) {}

        void OnSceneTransition() {}

        void Update(EntityId selectedEntity);
        void Draw(EntityId selectedEntity);

    private:
        TextEditor _textEditor;
        TransformValues _transform {};

        void BehaviorProperties(EntityId selectedEntity, SceneState& state);
        void TransformProperties(EntityId selectedEntity, SceneState& state);
        void UpdateTransformProperties(EntityId selectedEntity, SceneState& state);
    };
}  // namespace x::Editor