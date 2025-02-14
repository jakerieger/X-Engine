#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Volatile.hpp"

namespace x {
    class Camera final : public Volatile {
        VectorSet _position = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
        VectorSet _at       = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        VectorSet _up       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        VectorSet _forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        VectorSet _right   = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

        Float2 _rotation = {0.0f, 0.0f}; // pitch, yaw

        f32 _fovY        = XM_PIDIV4;
        f32 _aspectRatio = 16.f / 9.0f;
        f32 _nearZ       = 0.1f;
        f32 _farZ        = 1000.0f;

        Matrix _viewMatrix;
        Matrix _projectionMatrix;

    public:
        Camera();
        void OnResize(u32 width, u32 height) override;

        void Rotate(f32 deltaPitch, f32 deltaYaw);

        void SetPosition(const VectorSet& position);
        void SetFOV(f32 fovY);
        void SetAspectRatio(f32 ratio);
        void SetClipPlanes(f32 near, f32 far);

        f32 GetFovY() const {
            return _fovY;
        }

        f32 GetAspectRatio() const {
            return _aspectRatio;
        }

        std::pair<f32, f32> GetClipPlanes() const {
            return std::make_pair(_nearZ, _farZ);
        }

        [[nodiscard]] Matrix GetViewMatrix() const;
        [[nodiscard]] Matrix GetProjectionMatrix() const;
        [[nodiscard]] Matrix GetViewProjectionMatrix() const;
        [[nodiscard]] Float3 GetPosition() const;

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
    };
}