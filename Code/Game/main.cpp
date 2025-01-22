#include "Engine/Game.hpp"
#include "Common/Types.hpp"

using namespace x;

class SpaceGame final : public IGame {
public:
    SpaceGame() = default;
};

int main() {
    SpaceGame game;
    return 0;
}