// Author: Jake Rieger
// Created: 3/25/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Volatile.hpp"
#include "EngineCommon.hpp"

namespace x {
    class CameraComponent final : public Volatile {
    public:
        CameraComponent();

        // Setters
        CameraComponent& SetPosition(const Float3& position);
        CameraComponent& SetFOVDegrees(const f32 deg);
        CameraComponent& SetFOVRadians(const f32 rad);
        CameraComponent& SetAspectRatio(const f32 aspect);
        CameraComponent& SetNearPlane(const f32 nearZ);
        CameraComponent& SetFarPlane(const f32 farZ);
        CameraComponent& SetClipPlanes(const f32 nearZ, const f32 farZ);
        CameraComponent& SetOrthographic(const bool ortho = true);
        CameraComponent& SetWidth(const f32 width);
        CameraComponent& SetHeight(const f32 height);
        CameraComponent& SetWidthHeight(const f32 width, const f32 height);

        // Getters
        X_NODISCARD Float3 GetPosition() const;
        X_NODISCARD f32 GetFOVRadians() const;
        X_NODISCARD f32 GetFOVDegrees() const;
        X_NODISCARD f32 GetAspectRatio() const;
        X_NODISCARD f32 GetNearPlane() const;
        X_NODISCARD f32 GetFarPlane() const;
        X_NODISCARD Matrix GetViewMatrix() const;
        X_NODISCARD Matrix GetProjectionMatrix() const;
        void GetClipPlanes(f32& nearZ, f32& farZ) const;
        X_NODISCARD bool GetOrthographic() const;
        X_NODISCARD f32 GetWidth() const;
        X_NODISCARD f32 GetHeight() const;
        void GetWidthHeight(f32& width, f32& height) const;

        // Movement
        void Rotate(f32 deltaPitch, f32 deltaYaw);
        void MoveForward(f32 distance);
        void MoveRight(f32 distance);

        // from `Volatile` interface
        void OnResize(u32 width, u32 height) override;

    private:
        static constexpr f32 kDefaultAspect {16.0f / 9.0f};

        Float3 mPosition {0.0f, 0.0f, -5.0f};
        Float3 mLookAt {0.0f, 0.0f, 0.0f};
        Float3 mUp {0.0f, 1.0f, 0.0f};
        Float3 mForward {0.0f, 0.0f, 1.0f};
        Float3 mRight {1.0f, 0.0f, 0.0f};
        Float2 mRotation {0.0f, 0.0f};  // pitch and yaw
        f32 mFOV {XM_PIDIV4};           // Stored in radians!
        f32 mAspectRatio {kDefaultAspect};
        f32 mNearZ {0.01f};
        f32 mFarZ {1000.0f};
        Matrix mViewMatrix;
        Matrix mProjectionMatrix;

        // For ortho
        bool mOrthographic {false};  // Perspective camera by default
        f32 mWidth {1.0f};
        f32 mHeight {1.0f};

        void RecalculateViewMatrix();
        void RecalculateProjectionMatrix();
    };
}  // namespace x
