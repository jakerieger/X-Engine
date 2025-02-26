#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Volatile.hpp"

namespace x {
    class Camera final : public Volatile {
        VectorSet mPosition = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
        VectorSet mAt       = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        VectorSet mUp       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        VectorSet mForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        VectorSet mRight   = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

        Float2 mRotation = {0.0f, 0.0f};  // pitch, yaw

        f32 mFovY        = XM_PIDIV4;
        f32 mAspectRatio = 16.f / 9.0f;
        f32 mNearZ       = 0.1f;
        f32 mFarZ        = 1000.0f;

        Matrix mViewMatrix;
        Matrix mProjectionMatrix;

    public:
        Camera();
        void OnResize(u32 width, u32 height) override;

        void Rotate(f32 deltaPitch, f32 deltaYaw);
        void MoveForward(f32 distance);
        void MoveRight(f32 distance);

        void SetPosition(const VectorSet& position);
        void SetFOV(f32 fovY);
        void SetAspectRatio(f32 ratio);
        void SetClipPlanes(f32 nearZ, f32 farZ);

        f32 GetFovY() const {
            return mFovY;
        }

        f32 GetAspectRatio() const {
            return mAspectRatio;
        }

        std::pair<f32, f32> GetClipPlanes() const {
            return std::make_pair(mNearZ, mFarZ);
        }

        [[nodiscard]] Matrix GetViewMatrix() const;
        [[nodiscard]] Matrix GetProjectionMatrix() const;
        [[nodiscard]] Matrix GetViewProjectionMatrix() const;
        [[nodiscard]] Float3 GetPosition() const;

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
    };
}  // namespace x