#include "GameWindow.hpp"
#include "Engine/Game.hpp"

X_MAIN {
    GameWindow gameWindow("SpaceGame", "Scenes/Monke.xscn");
    gameWindow.Run();
}