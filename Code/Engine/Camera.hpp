#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Volatile.hpp"

namespace x {
    class Camera : public Volatile {
        VectorSet _position = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
        VectorSet _at       = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        VectorSet _up       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
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
        void SetFOV(f32 fovY);
        void SetAspectRatio(f32 ratio);
        void SetClipPlanes(f32 near, f32 far);

        Matrix GetViewMatrix() const;
        Matrix GetProjectionMatrix() const;
        Matrix GetViewProjectionMatrix() const;
        Float3 GetPosition() const;

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
    };
}