#include "GameWindow.hpp"
#include "Engine/Game.hpp"

X_MAIN {
    GameWindow gameWindow("SpaceGame", "Water");
    return gameWindow.Run();
}