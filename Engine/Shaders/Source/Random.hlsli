#ifndef RANDOM_HLSL
#define RANDOM_HLSL

/// Generate random unsigned integer from seed value
uint RandomU(uint seed) {
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d; // 666666669, prime number
    seed = seed ^ (seed >> 15);

    return seed;
}

/// Convert unsigned integer to normalized float value [0,1]
float UintToFloat(uint hash) {
    return float(hash & 0x7fffffff) / float(0x7fffffff);
}

/// Generate random float from seed
float RandomF(uint seed) {
    return UintToFloat(RandomU(seed));
}

/// Generate random float from 2D unsigned integer and seed
float RandomFrom2DF(uint2 uv, uint seed) {
    uint seed = uv.x + uv.y * 1597 + seed * 3571;
    return RandomF(seed);
}

#endif