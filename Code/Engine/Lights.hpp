// Author: Jake Rieger
// Created: 1/20/2025.
//

#pragma once

#include "Common/Types.hpp"
#include <DirectXMath.h>

namespace x {
    using float4 = DirectX::XMFLOAT4;
    using float3 = DirectX::XMFLOAT3;
    using float2 = DirectX::XMFLOAT2;

    static constexpr size_t kMaxPointLights = 16;
    static constexpr size_t kMaxSpotLights  = 16;
    static constexpr size_t kMaxAreaLights  = 16;

    struct alignas(16) DirectionalLight {
        float3 direction = {0.0f, 0.0f, 0.0f};
        float3 color     = {1.0f, 1.0f, 1.0f};
        f32 intensity    = 1.0f;
        bool castsShadow = true;
        bool enabled     = true;
    };

    struct alignas(16) PointLight {
        float3 position  = {0.0f, 0.0f, 0.0f};
        float3 color     = {1.0f, 1.0f, 1.0f};
        f32 intensity    = 1.0f;
        f32 constant     = 1.0f;
        f32 linear       = 0.09f;
        f32 quadratic    = 0.032f;
        f32 radius       = 45.f;
        bool castsShadow = true;
        bool enabled     = false;
    };

    struct alignas(16) SpotLight {
        float3 position  = {0.0f, 0.0f, 0.0f}; // Origin of the light
        float3 direction = {0.0f, -1.0f, 0.0f}; // Direction the cone points
        float3 color     = {1.0f, 1.0f, 1.0f}; // Light's color
        f32 intensity    = 1.0f; // Overall brightness
        f32 innerAngle   = 0.8f; // Inner cone angle (cosine)
        f32 outerAngle   = 0.6f; // Outer cone angle (cosine)
        f32 range        = 50.0f; // Maximum distance light travels
        bool castsShadow = true;
        bool enabled     = false;
    };

    struct alignas(16) AreaLight {
        float3 position   = {0.0f, 0.0f, 0.0f}; // Center position
        float3 direction  = {0.0f, -1.0f, 0.0f}; // Normal direction
        float3 color      = {1.0f, 1.0f, 1.0f}; // Light's color
        float2 dimensions = {1.0f, 1.0f}; // Width and height of the area
        f32 intensity     = 1.0f; // Overall brightness
        bool castsShadow  = true;
        bool enabled      = false;
    };

    struct LightState {
        DirectionalLight sun;
        PointLight pointLights[kMaxPointLights];
        SpotLight spotLights[kMaxSpotLights];
        AreaLight areaLights[kMaxAreaLights];
    };
}