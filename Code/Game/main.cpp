#include "GameWindow.hpp"
#include "Engine/Game.hpp"

// TODO: Include materials and scenes as part of pak file

X_MAIN {
    GameWindow gameWindow("SpaceGame", "Monke");
    return gameWindow.Run();
}