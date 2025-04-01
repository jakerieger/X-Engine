// Author: Jake Rieger
// Created: 4/1/2025.
//

#pragma once

#define X_ALIGN_MALLOC(size, align) _aligned_malloc(size, align)
#define X_ALIGN_FREE(ptr) _aligned_free(ptr)
#define X_KILOBYTES(n) ((size_t)(n) * 1024)
#define X_MEGABYTES(n) (KILOBYTES(n) * 1024)
#define X_GIGABYTES(n) (MEGABYTES(n) * 1024)

#ifdef _DEBUG
    #define X_DEBUG_ONLY(expr) expr

    #define X_ASSERT(cond)                                                                                             \
        if (!(cond)) __debugbreak();
#else
    #define DEBUG_ONLY(expr)

    #define X_ASSERT(cond)
#endif

#define X_STRINGIFY(x) #x
#define X_STRINGIFY_EXPAND(x) X_STRINGIFY(x)
#define X_CONCAT(a, b) a##b

#define X_BIT(x) (1ULL << (x))
#define X_SETBIT(x, bit) ((x) |= BIT(bit))
#define X_CLEARBIT(x, bit) ((x) &= ~BIT(bit))
#define X_TOGGLEBIT(x, bit) ((x) ^= BIT(bit))
#define X_CHECKBIT(x, bit) (!!((x) & BIT(bit)))

#define X_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define X_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define X_CLAMP(value, min, max) (MIN(MAX(value, min), max))

#define X_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define X_SAFE_DELETE(ptr)                                                                                             \
    do {                                                                                                               \
        if (ptr) {                                                                                                     \
            delete (ptr);                                                                                              \
            (ptr) = nullptr;                                                                                           \
        }                                                                                                              \
    } while (0)
#define X_SAFE_DELETE_ARRAY(ptr)                                                                                       \
    do {                                                                                                               \
        if (ptr) {                                                                                                     \
            delete[] (ptr);                                                                                            \
            (ptr) = nullptr;                                                                                           \
        }                                                                                                              \
    } while (0)

#define X_CACHE_ALIGNED __declspec(align(64))

#define X_NODISCARD [[nodiscard]]