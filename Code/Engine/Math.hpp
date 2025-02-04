#pragma once

#include "DirectXMath.h"
#include <intrin.h>

namespace x {
    using namespace DirectX;

    // DirectXMath Aliases
    using Matrix   = XMMATRIX;
    using Vector   = XMVECTOR;
    using Float4x4 = XMFLOAT4X4;
    using Float4   = XMFLOAT4;
    using Float3   = XMFLOAT3;
    using Float2   = XMFLOAT2;
    using Int4     = XMINT4;
    using Int3     = XMINT3;
    using Int2     = XMINT2;
    using Uint4    = XMUINT4;
    using Uint3    = XMUINT3;
    using Uint2    = XMUINT2;

    inline float Fractional(const float x) {
        return x - static_cast<float>(static_cast<int>(x));
    }
}