#include "Engine/Game.hpp"

X_MAIN {
    x::Game game(X_MODULE_HANDLE, "SpaceGame", 1600, 900);
    game.Run(R"(Scenes\monke.xscn)");
}