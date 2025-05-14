// Author: Jake Rieger
// Created: 4/16/2025.
//

#pragma once

#include "Typedefs.hpp"

namespace x {
    namespace Math {
        template<typename T>
        f64 Lerp(T a, T b, f64 t) {
            static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type");
            if (a == b) return a;
            return CAST<f64>(a * (1.0 - t) + b * t);
        }
    }  // namespace Math
}  // namespace x
