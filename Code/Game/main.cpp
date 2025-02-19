#include "GameWindow.hpp"
#include "Engine/Game.hpp"

// X_MAIN {
//     x::Game game(X_MODULE_HANDLE, "SpaceGame", 1600, 900);
//     game.Run(R"(Scenes\Monke.xscn)");
// }

X_MAIN {
    GameWindow gameWindow("SpaceGame", "Scenes/Monke.xscn");
    gameWindow.Run();
}