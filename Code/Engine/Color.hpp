// Author: Jake Rieger
// Created: 4/16/2025.
//

#pragma once

#include <imgui.h>
#include "D3D.hpp"
#include "Common/Types.hpp"
#include "Common/Macros.hpp"

namespace x {
    /// @brief Encapsulation for representing colors across the engine and its tools. Unifies ImGui, DirectX, and other
    /// API color usage and provides tons of utility for manipulation and blending.
    class Color {
    public:
        Color() = default;
        Color(f32 r, f32 g, f32 b, f32 a = 1.0f);

        explicit Color(f32 v, f32 a = 1.0f);
        explicit Color(u32 color);
        explicit Color(const str& hex);
        explicit Color(u8 r, u8 g, u8 b, u8 a = 255);
        explicit Color(const ImVec4& color);
        explicit Color(const XMFLOAT4& color);
        explicit Color(const f32* color);

        Color(const Color& other);
        Color& operator=(const Color& other);

        Color(Color&& other) noexcept;
        Color& operator=(Color&& other) noexcept;

        bool operator==(const Color& other) const;
        bool operator!=(const Color& other) const;

        // Component setters
        X_NODISCARD Color WithAlpha(f32 a) const;
        X_NODISCARD Color WithBlue(f32 b) const;
        X_NODISCARD Color WithGreen(f32 g) const;
        X_NODISCARD Color WithRed(f32 r) const;

        // Modifiers
        X_NODISCARD Color Brightness(f32 factor) const;
        X_NODISCARD Color Greyscale() const;
        X_NODISCARD Color Saturate(f32 factor) const;
        X_NODISCARD Color Desaturate(f32 factor) const;

        // Conversions
        X_NODISCARD ImVec4 ToImVec4() const;
        X_NODISCARD XMFLOAT4 ToXMFLOAT4() const;
        X_NODISCARD str ToString() const;
        X_NODISCARD u32 ToU32() const;
        X_NODISCARD u32 ToU32_ABGR() const;
        void ToFloatArray(f32* color) const;
        void ToHSV(f32& h, f32& s, f32& v) const;

        // Components
        X_NODISCARD f32 R() const;
        X_NODISCARD f32 G() const;
        X_NODISCARD f32 B() const;
        X_NODISCARD f32 A() const;
        X_NODISCARD f32 Luminance() const;

        // Static methods
        static Color AlphaBlend(const Color& foreground, const Color& background);
        static Color Lerp(const Color& a, const Color& b, f32 t);
        static Color Multiply(const Color& a, const Color& b);
        static Color Screen(const Color& a, const Color& b);
        static Color Overlay(const Color& a, const Color& b);
        static Color SoftLight(const Color& a, const Color& b);
        static Color HardLight(const Color& a, const Color& b);
        static Color ColorDodge(const Color& a, const Color& b);
        static Color ColorBurn(const Color& a, const Color& b);
        static Color FromHSV(f32 h, f32 s, f32 v, f32 a = 1.0f);

    private:
        f32 mRed {0.0f};
        f32 mGreen {0.0f};
        f32 mBlue {0.0f};
        f32 mAlpha {1.0f};

        /// @see
        /// https:stackoverflow.com/questions/61138110/what-is-the-correct-gamma-correction-function
        static f32 LinearizeComponent(f32 v);
        static u32 FloatToU32(f32 v);
        static f32 U32ToFloat(u32 v);
    };

    namespace Colors {
        static Color White {1.0f, 1.0f, 1.0f};
        static Color Black {0.0f, 0.0f, 0.0f};
        static Color Red {1.0f, 0.0f, 0.0f};
        static Color Green {0.0f, 1.0f, 0.0f};
        static Color Blue {0.0f, 0.0f, 1.0f};
        static Color Yellow {1.0f, 1.0f, 0.0f};
        static Color Magenta {1.0f, 0.0f, 1.0f};
        static Color Cyan {0.0f, 1.0f, 1.0f};
        static Color LightGrey {0.75f, 0.75f, 0.75f};
        static Color Grey {0.5f, 0.5f, 0.5f};
        static Color DarkGrey {0.25f, 0.25f, 0.25f};
        static Color White25 {1.0f, 1.0f, 1.0f, 0.25f};
        static Color White50 {1.0f, 1.0f, 1.0f, 0.5f};
        static Color White75 {1.0f, 1.0f, 1.0f, 0.75f};
        static Color Black25 {0.0f, 0.0f, 0.0f, 0.25f};
        static Color Black50 {0.0f, 0.0f, 0.0f, 0.5f};
        static Color Black75 {0.0f, 0.0f, 0.0f, 0.75f};
        static Color Transparent {0.0f, 0.0f};
    }  // namespace Colors
}  // namespace x

#ifndef X_COLOR_HASH_SPECIALIZATION
    #define X_COLOR_HASH_SPECIALIZATION
// Allow Color to be used as a key with STL maps/sets
template<>
struct std::hash<x::Color> {
    std::size_t operator()(const x::Color& color) const noexcept {
        return std::hash<x::u32> {}(color.ToU32());
    }
};  // namespace std
#endif
