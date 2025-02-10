#include "Camera.hpp"

namespace x {
    Camera::Camera() {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::OnResize(const u32 width, const u32 height) {
        _aspectRatio = CAST<f32>(width) / CAST<f32>(height);
        UpdateProjectionMatrix();
    }

    void Camera::SetPosition(const Vector& position) {
        _position = position;
        UpdateViewMatrix();
    }

    void Camera::SetFOV(const f32 fovY) {
        _fovY = XMConvertToRadians(fovY);
        UpdateProjectionMatrix();
    }

    void Camera::SetAspectRatio(const f32 ratio) {
        _aspectRatio = ratio;
        UpdateProjectionMatrix();
    }

    void Camera::SetClipPlanes(const f32 near, const f32 far) {
        _zNear = near;
        _zFar  = far;
        UpdateProjectionMatrix();
    }

    Matrix Camera::GetViewMatrix() const {
        return _viewMatrix;
    }

    Matrix Camera::GetProjectionMatrix() const {
        return _projectionMatrix;
    }

    Matrix Camera::GetViewProjectionMatrix() const {
        return XMMatrixMultiply(_viewMatrix, _projectionMatrix);
    }

    Float3 Camera::GetPosition() const {
        Float3 pos;
        XMStoreFloat3(&pos, _position);
        return pos;
    }

    void Camera::GetFrustumDimensions(f32& width, f32& height) const {
        height = 2.0f * _zFar * tanf(_fovY * 0.5f);
        width  = height * _aspectRatio;
    }

    f32 Camera::GetSceneRadius() const {
        f32 width, height;
        GetFrustumDimensions(width, height);

        if (width <= 0 || height <= 0)
            return std::numeric_limits<f32>::infinity();

        return sqrtf((width * width) + (height * height) + (_zFar * _zFar)) * 0.5f;
    }

    void Camera::UpdateViewMatrix() {
        _viewMatrix = XMMatrixLookAtLH(_position, _at, _up);
    }

    void Camera::UpdateProjectionMatrix() {
        _projectionMatrix = XMMatrixPerspectiveFovLH(_fovY, _aspectRatio, _zNear, _zFar);
    }
}