#include "Camera.hpp"
#include "EngineCommon.hpp"

namespace x {
    Camera::Camera() {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::OnResize(const u32 width, const u32 height) {
        X_DEBUG_LOG_RESIZE("Camera", width, height)
        _aspectRatio = CAST<f32>(width) / CAST<f32>(height);
        UpdateProjectionMatrix();
    }

    void Camera::Rotate(f32 deltaPitch, f32 deltaYaw) {
        _rotation.x += deltaPitch;
        _rotation.y += deltaYaw;

        constexpr f32 maxPitch = XM_PIDIV2 - 0.01f;  // clamp pitch to prevent flipping
        _rotation.x            = std::max(-maxPitch, std::min(maxPitch, _rotation.x));

        // Keep yaw in [0, 2PI] range
        if (_rotation.y > XM_2PI) {
            _rotation.y -= XM_2PI;
        } else if (_rotation.y < 0.0f) {
            _rotation.y += XM_2PI;
        }

        Matrix rotationMatrix = XMMatrixRotationRollPitchYaw(_rotation.x, _rotation.y, 0.0f);

        VectorSet defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        VectorSet defaultRight   = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        VectorSet defaultUp      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        _forward = XMVector3TransformCoord(defaultForward, rotationMatrix);
        _right   = XMVector3TransformCoord(defaultRight, rotationMatrix);
        _up      = XMVector3TransformCoord(defaultUp, rotationMatrix);

        // Update look-at point
        _at = XMVectorAdd(_position, _forward);

        UpdateViewMatrix();
    }

    void Camera::MoveForward(f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_forward, distance));
        _at       = XMVectorAdd(_position, _forward);
        UpdateViewMatrix();
    }

    void Camera::MoveRight(f32 distance) {
        _position = XMVectorAdd(_position, XMVectorScale(_right, distance));
        _at       = XMVectorAdd(_position, _forward);
        UpdateViewMatrix();
    }

    void Camera::SetPosition(const VectorSet& position) {
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

    void Camera::SetClipPlanes(const f32 nearZ, const f32 farZ) {
        _nearZ = nearZ;
        _farZ  = farZ;
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

    void Camera::UpdateViewMatrix() {
        _viewMatrix = XMMatrixLookAtLH(_position, _at, _up);
    }

    void Camera::UpdateProjectionMatrix() {
        _projectionMatrix = XMMatrixPerspectiveFovLH(_fovY, _aspectRatio, _nearZ, _farZ);
    }
}  // namespace x