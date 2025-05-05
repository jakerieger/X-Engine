// Author: Jake Rieger
// Created: 4/16/2025.
//

#pragma warning(disable : 4244)

#include <iomanip>
#include <sstream>
#include "Color.hpp"
#include "../Common/Templates.hpp"

namespace x {
    Color::Color(f32 v, f32 a) {
        mRed   = v;
        mGreen = v;
        mBlue  = v;
        mAlpha = a;
    }

    Color::Color(f32 r, f32 g, f32 b, f32 a) {
        mRed   = r;
        mGreen = g;
        mBlue  = b;
        mAlpha = a;
    }

    Color::Color(u32 color) {
        mAlpha = (color >> 24) & 0xFF;
        mRed   = (color >> 16) & 0xFF;
        mGreen = (color >> 8) & 0xFF;
        mBlue  = color & 0xFF;
    }

    Color::Color(const str& hex) {
        if (hex.length() != 7) {
            throw std::invalid_argument("Invalid hex string, incorrect length. Must be in format '#RRGGBB'");
        }

        const str colors = hex.substr(1, hex.length() - 1);  // Remove pound (#) sign
        const u32 r      = std::strtoul(colors.substr(0, 2).c_str(), nullptr, 16);
        const u32 g      = std::strtoul(colors.substr(2, 2).c_str(), nullptr, 16);
        const u32 b      = std::strtoul(colors.substr(4, 2).c_str(), nullptr, 16);

        mRed   = U32ToFloat(r);
        mGreen = U32ToFloat(g);
        mBlue  = U32ToFloat(b);
        mAlpha = 1.0f;
    }

    Color::Color(u8 r, u8 g, u8 b, u8 a) {
        mRed   = r / 255.0f;
        mGreen = g / 255.0f;
        mBlue  = b / 255.0f;
        mAlpha = a / 255.0f;
    }

    Color::Color(const ImVec4& color) {
        mRed   = color.x;
        mGreen = color.y;
        mBlue  = color.z;
        mAlpha = color.w;
    }

    Color::Color(const XMFLOAT4& color) {
        mRed   = color.x;
        mGreen = color.y;
        mBlue  = color.z;
        mAlpha = color.w;
    }

    Color::Color(const f32* color) {
        if (color == nullptr) { throw std::invalid_argument("Invalid color ptr, nullptr"); }
        mRed   = color[0];
        mGreen = color[1];
        mBlue  = color[2];
        mAlpha = color[3];
    }

    Color::Color(const Color& other)
        : mRed(other.mRed), mGreen(other.mGreen), mBlue(other.mBlue), mAlpha(other.mAlpha) {}

    Color& Color::operator=(const Color& other) {
        if (this != &other) {
            mRed   = other.mRed;
            mGreen = other.mGreen;
            mBlue  = other.mBlue;
            mAlpha = other.mAlpha;
        }
        return *this;
    }

    Color::Color(Color&& other) noexcept
        : mRed(std::exchange(other.mRed, 0.0f)), mGreen(std::exchange(other.mGreen, 0.0f)),
          mBlue(std::exchange(other.mBlue, 0.0f)), mAlpha(std::exchange(other.mAlpha, 1.0f)) {}

    Color& Color::operator=(Color&& other) noexcept {
        if (this != &other) {
            mRed   = std::exchange(other.mRed, 0.0f);
            mGreen = std::exchange(other.mGreen, 0.0f);
            mBlue  = std::exchange(other.mBlue, 0.0f);
            mAlpha = std::exchange(other.mAlpha, 1.0f);
        }
        return *this;
    }

    bool Color::operator==(const Color& other) const {
        return mRed == other.mRed && mGreen == other.mGreen && mBlue == other.mBlue && mAlpha == other.mAlpha;
    }

    bool Color::operator!=(const Color& other) const {
        return !(*this == other);
    }

    Color Color::WithAlpha(f32 a) const {
        return Color(mRed, mGreen, mBlue, a);
    }

    Color Color::WithBlue(f32 b) const {
        return Color(mRed, mGreen, b, mAlpha);
    }

    Color Color::WithGreen(f32 g) const {
        return Color(mRed, g, mBlue, mAlpha);
    }

    Color Color::WithRed(f32 r) const {
        return Color(r, mGreen, mBlue, mAlpha);
    }

    Color Color::Brightness(f32 factor) const {
        // Clamp factor to prevent extreme values
        factor = X_MIN(0.0f, factor);

        return {
          X_MIN(1.0f, mRed * factor),
          X_MIN(1.0f, mGreen * factor),
          X_MIN(1.0f, mBlue * factor),
          mAlpha,
        };
    }

    Color Color::Greyscale() const {
        return Color(Luminance(), mAlpha);
    }

    f32 Color::Luminance() const {
        const auto R = LinearizeComponent(mRed);
        const auto G = LinearizeComponent(mGreen);
        const auto B = LinearizeComponent(mBlue);
        return 0.2126f * R + 0.7152f * G + 0.0722f * B;
    }

    Color Color::Saturate(f32 factor) const {
        f32 h, s, v;
        ToHSV(h, s, v);
        s = X_MIN(1.0f, s * factor);
        return FromHSV(h, s, v, mAlpha);
    }

    Color Color::Desaturate(f32 factor) const {
        f32 h, s, v;
        ToHSV(h, s, v);
        s = X_MIN(0.0f, s / factor);
        return FromHSV(h, s, v, mAlpha);
    }

    ImVec4 Color::ToImVec4() const {
        return ImVec4(mRed, mGreen, mBlue, mAlpha);
    }

    XMFLOAT4 Color::ToXMFLOAT4() const {
        return XMFLOAT4(mRed, mGreen, mBlue, mAlpha);
    }

    str Color::ToString() const {
        const u32 r = U32ToFloat(mRed + 0.5f);
        const u32 g = U32ToFloat(mGreen + 0.5f);
        const u32 b = U32ToFloat(mBlue + 0.5f);
        std::stringstream ss;
        ss << "#" << std::hex << std::setfill('0') << std::setw(2) << (r & 0xFF) << std::setw(2) << (g & 0xFF)
           << std::setw(2) << (b & 0xFF);
        return ss.str();
    }

    u32 Color::ToU32() const {
        const auto redByte   = CAST<u8>(FloatToU32(mRed));
        const auto greenByte = CAST<u8>(FloatToU32(mGreen));
        const auto blueByte  = CAST<u8>(FloatToU32(mBlue));
        const auto alphaByte = CAST<u8>(FloatToU32(mAlpha));
        return (alphaByte << 24) | (redByte << 16) | (greenByte << 8) | blueByte;
    }

    /// For ImGui 32-bit unsigned color representations
    u32 Color::ToU32_ABGR() const {
        const auto redByte   = CAST<u8>(FloatToU32(mRed));
        const auto greenByte = CAST<u8>(FloatToU32(mGreen));
        const auto blueByte  = CAST<u8>(FloatToU32(mBlue));
        const auto alphaByte = CAST<u8>(FloatToU32(mAlpha));
        return (alphaByte << 24) | (blueByte << 16) | (greenByte << 8) | redByte;
    }

    void Color::ToFloatArray(f32* color) const {
        color[0] = mRed;
        color[1] = mGreen;
        color[2] = mBlue;
        color[3] = mAlpha;
    }

    void Color::ToHSV(f32& h, f32& s, f32& v) const {
        const f32 max   = X_MAX(X_MAX(mRed, mGreen), mBlue);
        const f32 min   = X_MIN(X_MIN(mRed, mGreen), mBlue);
        const f32 delta = max - min;

        v = max;
        s = (max > 0.0f) ? (delta / max) : 0.0f;

        if (delta < 0.00001f) {
            h = 0.0f;  // No saturation, so hue is undefined
        } else {
            if (mRed >= max) {
                h = (mGreen - mBlue) / delta;
            } else if (mGreen >= max) {
                h = 2.0f + (mBlue - mRed) / delta;
            } else {
                h = 4.0f + (mRed - mGreen) / delta;
            }

            h *= 60.0f;  // Convert to degrees
            if (h < 0.0f) { h += 360.0f; }
            h /= 360.0f;  // Normalize to [0, 1]
        }
    }

    Color Color::FromHSV(f32 h, f32 s, f32 v, f32 a) {
        if (s <= 0.0f) { return Color(v, a); }

        h = std::fmod(h, 1.0f) * 6.0f;  // Map h to [0, 6)

        const int i = CAST<int>(h);
        const f32 f = h - i;
        const f32 p = v * (1.0f - s);
        const f32 q = v * (1.0f - s * f);
        const f32 t = v * (1.0f - s * (1.0f - f));

        switch (i) {
            case 0:
                return Color(v, t, p, a);
            case 1:
                return Color(q, v, p, a);
            case 2:
                return Color(p, v, t, a);
            case 3:
                return Color(p, q, v, a);
            case 4:
                return Color(t, p, v, a);
            default:
                return Color(v, p, q, a);
        }
    }

    f32 Color::R() const {
        return mRed;
    }

    f32 Color::G() const {
        return mGreen;
    }

    f32 Color::B() const {
        return mBlue;
    }

    f32 Color::A() const {
        return mAlpha;
    }

    Color Color::AlphaBlend(const Color& foreground, const Color& background) {
        const f32 alpha = foreground.A();
        if (alpha == 0.f) { return background; }
        const f32 invAlpha = 1.f - alpha;

        f32 backAlpha = background.A();
        if (backAlpha == 1.f) {
            return {alpha * foreground.R() + invAlpha * background.R(),
                    alpha * foreground.G() + invAlpha * background.G(),
                    alpha * foreground.B() + invAlpha * background.B(),
                    1.f};
        }

        backAlpha          = (backAlpha * invAlpha) / 1.f;
        const f32 outAlpha = alpha + backAlpha;
        X_ASSERT(outAlpha != 0.f);
        return {alpha * foreground.R() + invAlpha * background.R() / outAlpha,
                alpha * foreground.G() + invAlpha * background.G() / outAlpha,
                alpha * foreground.B() + invAlpha * background.B() / outAlpha,
                1.f};
    }

    Color Color::Lerp(const Color& a, const Color& b, f32 t) {
        return {(f32)Math::Lerp(a.mRed, b.mRed, t),
                (f32)Math::Lerp(a.mGreen, b.mGreen, t),
                (f32)Math::Lerp(a.mBlue, b.mBlue, t),
                (f32)Math::Lerp(a.mAlpha, b.mAlpha, t)};
    }

    Color Color::Multiply(const Color& a, const Color& b) {
        return {a.R() * b.R(), a.G() * b.G(), a.B() * b.B(), a.A() * b.A()};
    }

    Color Color::Screen(const Color& a, const Color& b) {
        return {1.0f - (1.0f - a.R()) * (1.0f - b.R()),
                1.0f - (1.0f - a.G()) * (1.0f - b.G()),
                1.0f - (1.0f - a.B()) * (1.0f - b.B()),
                a.A() * b.A()};
    }

    Color Color::Overlay(const Color& a, const Color& b) {
        auto overlayComponent = [](f32 a, f32 b) -> f32 {
            if (a < 0.5f) {
                return 2.0f * a * b;
            } else {
                return 1.0f - 2.0f * (1.0f - a) * (1.0f - b);
            }
        };

        return {overlayComponent(a.R(), b.R()),
                overlayComponent(a.G(), b.G()),
                overlayComponent(a.B(), b.B()),
                a.A() * b.A()};
    }

    Color Color::SoftLight(const Color& a, const Color& b) {
        auto softLightComponent = [](f32 a, f32 b) -> f32 {
            if (b < 0.5f) {
                return a - (1.0f - 2.0f * b) * a * (1.0f - a);
            } else {
                f32 d;
                if (a < 0.25f) {
                    d = ((16.0f * a - 12.0f) * a + 4.0f) * a;
                } else {
                    d = std::sqrt(a);
                }
                return a + (2.0f * b - 1.0f) * (d - a);
            }
        };

        return {softLightComponent(a.R(), b.R()),
                softLightComponent(a.G(), b.G()),
                softLightComponent(a.B(), b.B()),
                a.A() * b.A()};
    }

    Color Color::HardLight(const Color& a, const Color& b) {
        // Hard light is essentially Overlay with parameters swapped
        return Overlay(b, a);
    }

    Color Color::ColorDodge(const Color& a, const Color& b) {
        auto colorDodgeComponent = [](f32 a, f32 b) -> f32 {
            if (b == 1.0f) {
                return 1.0f;
            } else {
                return X_MIN(1.0f, a / (1.0f - b));
            }
        };

        return {colorDodgeComponent(a.R(), b.R()),
                colorDodgeComponent(a.G(), b.G()),
                colorDodgeComponent(a.B(), b.B()),
                a.A() * b.A()};
    }

    Color Color::ColorBurn(const Color& a, const Color& b) {
        auto colorBurnComponent = [](f32 a, f32 b) -> f32 {
            if (b == 0.0f) {
                return 0.0f;
            } else {
                return 1.0f - X_MIN(1.0f, (1.0f - a) / b);
            }
        };

        return {colorBurnComponent(a.R(), b.R()),
                colorBurnComponent(a.G(), b.G()),
                colorBurnComponent(a.B(), b.B()),
                a.A() * b.A()};
    }

    f32 Color::LinearizeComponent(f32 v) {
        if (v <= 0.04045f) { return v / 12.92f; }
        return std::pow((v + 0.055f) / 1.055f, 2.4f);
    }

    u32 Color::FloatToU32(f32 v) {
        return CAST<u32>(v * 255.0f);
    }

    f32 Color::U32ToFloat(u32 v) {
        return v / 255.0f;
    }
}  // namespace x