#include "Camera.hpp"

namespace x {
    Camera::Camera() {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::OnResize(const u32 width, const u32 height) {
        _aspectRatio = CAST<f32>(width) / CAST<f32>(height);
    }

    void Camera::SetPosition(const VectorSet& position) {
        _position = position;
        UpdateViewMatrix();
    }

    void Camera::MoveForward(const f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_forward, distance));
        UpdateViewMatrix();
    }

    void Camera::MoveRight(const f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_right, distance));
        UpdateViewMatrix();
    }

    void Camera::MoveUp(const f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_up, distance));
        UpdateViewMatrix();
    }

    void Camera::Rotate(const f32 pitch, const f32 yaw, const f32 roll) {
        const XMMATRIX rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

        _forward = XMVector3TransformNormal(_forward, rotation);
        _up      = XMVector3TransformNormal(_up, rotation);
        _right   = XMVector3TransformNormal(_right, rotation);

        _forward = XMVector3Normalize(_forward);
        _right   = XMVector3Cross(_up, _forward);
        _up      = XMVector3Cross(_forward, _right);

        UpdateViewMatrix();
    }

    void Camera::LookAt(const VectorSet& target) {
        _forward = XMVector3Normalize(XMVectorSubtract(target, _position));
        _right   = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), _forward));
        _up      = XMVector3Cross(_forward, _right);

        UpdateViewMatrix();
    }

    void Camera::SetFOV(const f32 fovY) {
        _fovY = fovY;
    }

    void Camera::SetAspectRatio(const f32 ratio) {
        _aspectRatio = ratio;
    }

    void Camera::SetClipPlanes(const f32 near, const f32 far) {
        _zNear = near;
        _zFar  = far;
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

    void Camera::UpdateViewMatrix() {
        _viewMatrix = XMMatrixLookToLH(_position, _forward, _up);
    }

    void Camera::UpdateProjectionMatrix() {
        _projectionMatrix = XMMatrixPerspectiveFovLH(_fovY, _aspectRatio, _zNear, _zFar);
    }
}