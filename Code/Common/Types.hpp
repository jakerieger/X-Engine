// Author: Jake Rieger
// Created: 1/7/2025.
//

#pragma once
#pragma warning(disable : 4267)

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

namespace x {
    using u8   = uint8_t;
    using u16  = uint16_t;
    using u32  = uint32_t;
    using u64  = uint64_t;
    using uptr = uintptr_t;

    using i8   = int8_t;
    using i16  = int16_t;
    using i32  = int32_t;
    using i64  = int64_t;
    using iptr = intptr_t;

#if defined(__GNUC__) || defined(__clang__)
    using u128 = __uint128_t;
    using i128 = __int128_t;
#endif

    using f32 = float;
    using f64 = double;

    using cstr    = const char*;
    using str     = std::string;
    using wstr    = std::wstring;
    using strview = std::string_view;

    inline constexpr auto None            = nullptr;
    inline constexpr std::nullopt_t Empty = std::nullopt;
    inline constexpr std::_Ignore _i_     = std::ignore;

    // Most used STL objects included for convenience
    using std::array;
    using std::make_shared;
    using std::make_unique;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::unordered_map;
    using std::vector;
    using std::weak_ptr;

    // Static assert and concept aliases
    template<class A, class B>
    inline constexpr bool Same = std::is_same_v<A, B>;

    template<class B, class D>
    inline constexpr bool BaseOf = std::is_base_of_v<B, D>;

#define CAST static_cast
#define CCAST const_cast
#define DCAST dynamic_cast
#define RCAST reinterpret_cast

#define CHECK_FLAG(bits, flag) (bits & flag) != 0
}  // namespace x

constexpr auto HLSL_TRUE  = 1U;
constexpr auto HLSL_FALSE = 0U;