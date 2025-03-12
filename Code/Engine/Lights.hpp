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
        Float4 mDirection     = {0.0f, 0.0f, 0.0f, 1.0f};  // 16 bytes
        Float4 mColor         = {1.0f, 1.0f, 1.0f, 1.0f};  // 16 bytes
        f32 mIntensity        = 1.0f;                      // 4 bytes
        f32 mPad1[3]          = {0.0f, 0.0f, 0.0f};
        u32 mCastsShadows     = HLSL_TRUE;  // 4 bytes
        u32 mEnabled          = HLSL_TRUE;  // 4 bytes
        f32 mPad2[2]          = {0.0f, 0.0f};
        Matrix mLightViewProj = XMMatrixIdentity();
    };

    struct alignas(16) PointLight {
        Float3 mPosition = {0.0f, 0.0f, 0.0f};
        f32 mPad1        = 0.f;
        Float3 mColor    = {1.0f, 1.0f, 1.0f};
        f32 mIntensity   = 1.0f;
        f32 mConstant    = 1.0f;
        f32 mLinear      = 0.09f;
        f32 mQuadratic   = 0.032f;
        f32 mRadius      = 45.f;
        f32 mPad2        = 0.f;
        u32 mCastsShadow = HLSL_TRUE;
        u32 mEnabled     = HLSL_FALSE;
    };

    struct alignas(16) SpotLight {
        Float3 mPosition  = {0.0f, 0.0f, 0.0f};  // Origin of the light
        f32 mPad1         = 0.f;
        Float3 mDirection = {0.0f, -1.0f, 0.0f};  // Direction the cone points
        f32 mPad2         = 0.f;
        Float3 mColor     = {1.0f, 1.0f, 1.0f};  // Light's color
        f32 mIntensity    = 1.0f;                // Overall brightness
        f32 mInnerAngle   = 0.8f;                // Inner cone angle (cosine)
        f32 mOuterAngle   = 0.6f;                // Outer cone angle (cosine)
        f32 mRange        = 50.0f;               // Maximum distance light travels
        u32 mCastsShadow  = HLSL_TRUE;
        u32 mEnabled      = HLSL_FALSE;
        f32 mPad3[3]      = {0.0f, 0.0f, 0.0f};
    };

    struct alignas(16) AreaLight {
        Float3 mPosition = {0.0f, 0.0f, 0.0f};  // Center position
        f32 mPad1;
        Float3 mDirection = {0.0f, -1.0f, 0.0f};  // Normal direction
        f32 mPad2;
        Float3 mColor = {1.0f, 1.0f, 1.0f};  // Light's color
        f32 mPad3;
        Float2 mDimensions = {1.0f, 1.0f};  // Width and height of the area
        f32 mIntensity     = 1.0f;          // Overall brightness
        u32 mCastsShadow   = HLSL_TRUE;
        u32 mEnabled       = HLSL_FALSE;
        f32 mPad4[3];
    };

    struct LightState {
        DirectionalLight mSun;
        PointLight mPointLights[kMaxPointLights];
        SpotLight mSpotLights[kMaxSpotLights];
        AreaLight mAreaLights[kMaxAreaLights];
    };

    inline Matrix CalculateLightViewProjection(const DirectionalLight& light,
                                               f32 viewWidth,
                                               f32 aspectRatio,
                                               f32 nearZ,
                                               f32 farZ,
                                               const Float3& sceneCenter = {0.f, 0.f, 0.f}) {
        if (light.mDirection.x == 0.f && light.mDirection.y == 0.f && light.mDirection.z == 0.f) {
            return XMMatrixIdentity();  // Prevent EyePosition in XMMatrixLookAtLH from being zero (which will crash)
        }

        VectorSet direction = XMLoadFloat4(&light.mDirection);
        direction           = XMVector3Normalize(direction);

        const auto viewHeight         = viewWidth / aspectRatio;
        const VectorSet center        = XMLoadFloat3(&sceneCenter);
        const VectorSet lightPosition = center + (direction * viewHeight);
        VectorSet worldUp             = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        if (Abs(XMVectorGetY(direction)) > 0.99f) { worldUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); }

        const Matrix lightView = XMMatrixLookAtLH(lightPosition, center, worldUp);
        const Matrix lightProj = XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);

        return lightView * lightProj;
    }
}  // namespace x