// Author: Jake Rieger
// Created: 1/20/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Camera.hpp"

namespace x {
    inline constexpr size_t kMaxPointLights = 16;
    inline constexpr size_t kMaxSpotLights  = 16;
    inline constexpr size_t kMaxAreaLights  = 16;

    struct alignas(16) DirectionalLight {
        Float4 direction     = {0.0f, 0.0f, 0.0f, 1.0f};  // 16 bytes
        Float4 color         = {1.0f, 1.0f, 1.0f, 1.0f};  // 16 bytes
        f32 intensity        = 1.0f;                      // 4 bytes
        f32 mPad1[3]         = {0.0f, 0.0f, 0.0f};
        u32 castsShadows     = HLSL_TRUE;  // 4 bytes
        u32 enabled          = HLSL_TRUE;  // 4 bytes
        f32 mPad2[2]         = {0.0f, 0.0f};
        Matrix lightViewProj = XMMatrixIdentity();
    };

    struct alignas(16) PointLight {
        Float3 position = {0.0f, 0.0f, 0.0f};
        f32 mPad1       = 0.f;
        Float3 color    = {1.0f, 1.0f, 1.0f};
        f32 intensity   = 1.0f;
        f32 constant    = 1.0f;
        f32 linear      = 0.09f;
        f32 quadratic   = 0.032f;
        f32 radius      = 45.f;
        f32 mPad2       = 0.f;
        u32 castsShadow = HLSL_TRUE;
        u32 enabled     = HLSL_FALSE;
    };

    struct alignas(16) SpotLight {
        Float3 position  = {0.0f, 0.0f, 0.0f};  // Origin of the light
        f32 mPad1        = 0.f;
        Float3 direction = {0.0f, -1.0f, 0.0f};  // Direction the cone points
        f32 mPad2        = 0.f;
        Float3 color     = {1.0f, 1.0f, 1.0f};  // Light's color
        f32 intensity    = 1.0f;                // Overall brightness
        f32 innerAngle   = 0.8f;                // Inner cone angle (cosine)
        f32 outerAngle   = 0.6f;                // Outer cone angle (cosine)
        f32 range        = 50.0f;               // Maximum distance light travels
        u32 castsShadow  = HLSL_TRUE;
        u32 enabled      = HLSL_FALSE;
        f32 mPad3[3]     = {0.0f, 0.0f, 0.0f};
    };

    struct alignas(16) AreaLight {
        Float3 position = {0.0f, 0.0f, 0.0f};  // Center position
        f32 mPad1;
        Float3 direction = {0.0f, -1.0f, 0.0f};  // Normal direction
        f32 mPad2;
        Float3 color = {1.0f, 1.0f, 1.0f};  // Light's color
        f32 mPad3;
        Float2 dimensions = {1.0f, 1.0f};  // Width and height of the area
        f32 intensity     = 1.0f;          // Overall brightness
        u32 castsShadow   = HLSL_TRUE;
        u32 enabled       = HLSL_FALSE;
        f32 mPad4[3];
    };

    struct LightState {
        DirectionalLight Sun;
        PointLight PointLights[kMaxPointLights];
        SpotLight SpotLights[kMaxSpotLights];
        AreaLight AreaLights[kMaxAreaLights];
    };

    inline Matrix CalculateLightViewProjection(const DirectionalLight& light,
                                               f32 viewWidth,
                                               f32 aspectRatio,
                                               f32 nearZ,
                                               f32 farZ,
                                               const Float3& sceneCenter = {0.f, 0.f, 0.f}) {
        if (light.direction.x == 0.f && light.direction.y == 0.f && light.direction.z == 0.f) {
            return XMMatrixIdentity();  // Prevent EyePosition in XMMatrixLookAtLH from being zero (which will crash)
        }

        VectorSet direction = XMLoadFloat4(&light.direction);
        direction           = XMVector3Normalize(direction);

        const auto viewHeight   = viewWidth / aspectRatio;
        VectorSet center        = XMLoadFloat3(&sceneCenter);
        VectorSet lightPosition = center + (direction * viewHeight);
        VectorSet worldUp       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        if (Abs(XMVectorGetY(direction)) > 0.99f) { worldUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); }

        Matrix lightView = XMMatrixLookAtLH(lightPosition, center, worldUp);
        Matrix lightProj = XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);

        return lightView * lightProj;
    }
}  // namespace x