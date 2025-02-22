// Author: Jake Rieger
// Created: 2/21/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/EntityId.hpp"

namespace x {
    class Game;
}

namespace x::Editor {
    struct TransformValues {
        f32 position[3];
        f32 rotation[3];
        f32 scale[3];
    };

    class PropertiesPanel {
        Game& _game;

    public:
        explicit PropertiesPanel(Game& game) : _game(game) {}

        void OnSceneTransition() {}

        void Update(EntityId selectedEntity);
        void Draw(EntityId selectedEntity);

    private:
        TransformValues _transform {};
    };
}  // namespace x::Editor