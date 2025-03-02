// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "IdGen.hpp"
#include <random>

namespace x {
    u64 IdGen::GenerateId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<u64> dis(0, std::numeric_limits<u64>::max());
        return dis(gen);
    }
}  // namespace x