#pragma once

#include <bit>
#include "DirectXMath.h"

namespace x {
    using namespace DirectX;

    // DirectXMath Aliases
    using Matrix    = XMMATRIX;
    using VectorSet = XMVECTOR;
    using Float4x4  = XMFLOAT4X4;
    using Float4    = XMFLOAT4;
    using Float3    = XMFLOAT3;
    using Float2    = XMFLOAT2;
    using Int4      = XMINT4;
    using Int3      = XMINT3;
    using Int2      = XMINT2;
    using Uint4     = XMUINT4;
    using Uint3     = XMUINT3;
    using Uint2     = XMUINT2;

    inline VectorSet Float3ToVectorSet(const Float3& v) {
        return XMLoadFloat3(&v);
    }

    inline Float3 VectorSetToFloat3(const VectorSet& v) {
        Float3 dest;
        XMStoreFloat3(&dest, v);
        return dest;
    }

    inline float Fractional(const float x) {
        return x - static_cast<float>(static_cast<int>(x));
    }

    inline float Abs(const float x) {
        if (x < 0.0f) return -x;
        return x;
    }
}  // namespace x