// Author: Jake Rieger
// Created: 1/20/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"

namespace x {
    static constexpr size_t kMaxPointLights = 16;
    static constexpr size_t kMaxSpotLights  = 16;
    static constexpr size_t kMaxAreaLights  = 16;

    struct alignas(16) DirectionalLight {
        Float3 direction = {0.0f, 0.0f, 0.0f};
        Float3 color     = {1.0f, 1.0f, 1.0f};
        f32 intensity    = 1.0f;
        bool castsShadow = true;
        bool enabled     = true;
    };

    struct alignas(16) PointLight {
        Float3 position  = {0.0f, 0.0f, 0.0f};
        Float3 color     = {1.0f, 1.0f, 1.0f};
        f32 intensity    = 1.0f;
        f32 constant     = 1.0f;
        f32 linear       = 0.09f;
        f32 quadratic    = 0.032f;
        f32 radius       = 45.f;
        bool castsShadow = true;
        bool enabled     = false;
    };

    struct alignas(16) SpotLight {
        Float3 position  = {0.0f, 0.0f, 0.0f}; // Origin of the light
        Float3 direction = {0.0f, -1.0f, 0.0f}; // Direction the cone points
        Float3 color     = {1.0f, 1.0f, 1.0f}; // Light's color
        f32 intensity    = 1.0f; // Overall brightness
        f32 innerAngle   = 0.8f; // Inner cone angle (cosine)
        f32 outerAngle   = 0.6f; // Outer cone angle (cosine)
        f32 range        = 50.0f; // Maximum distance light travels
        bool castsShadow = true;
        bool enabled     = false;
    };

    struct alignas(16) AreaLight {
        Float3 position   = {0.0f, 0.0f, 0.0f}; // Center position
        Float3 direction  = {0.0f, -1.0f, 0.0f}; // Normal direction
        Float3 color      = {1.0f, 1.0f, 1.0f}; // Light's color
        Float2 dimensions = {1.0f, 1.0f}; // Width and height of the area
        f32 intensity     = 1.0f; // Overall brightness
        bool castsShadow  = true;
        bool enabled      = false;
    };

    struct LightState {
        DirectionalLight Sun;
        PointLight PointLights[kMaxPointLights];
        SpotLight SpotLights[kMaxSpotLights];
        AreaLight AreaLights[kMaxAreaLights];
    };
}