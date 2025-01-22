#include <iostream>
#include "Engine/Game.hpp"

using namespace x;

class SpaceGame final : public IGame {
public:
    SpaceGame() = default;
};

int main() {
    SpaceGame game;

    const uptr x = RCAST<uptr>(&game);
    std::cout << x << '\n';

    return 0;
}