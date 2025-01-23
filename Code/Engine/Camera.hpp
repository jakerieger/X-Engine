#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Volatile.hpp"

namespace x {
    class Camera : public Volatile {
        VectorSet _position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        VectorSet _forward  = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
        VectorSet _up       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        VectorSet _right    = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        f32 _fovY           = XM_PIDIV4;
        f32 _aspectRatio    = 16.f / 9.0f;
        f32 _zNear          = 0.1f;
        f32 _zFar           = 1000.0f;
        Matrix _viewMatrix;
        Matrix _projectionMatrix;

    public:
        Camera();
        void OnResize(u32 width, u32 height) override;

        void SetPosition(const VectorSet& position);
        void MoveForward(f32 distance);
        void MoveRight(f32 distance);
        void MoveUp(f32 distance);
        void Rotate(f32 pitch, f32 yaw, f32 roll);
        void LookAt(const VectorSet& target);
        void SetFOV(f32 fovY);
        void SetAspectRatio(f32 ratio);
        void SetClipPlanes(f32 near, f32 far);

        Matrix GetViewMatrix() const;
        Matrix GetProjectionMatrix() const;
        Matrix GetViewProjectionMatrix() const;

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
    };
}