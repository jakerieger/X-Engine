#include "Camera.hpp"
#include "EngineCommon.hpp"

namespace x {
    Camera::Camera() {
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }

    void Camera::OnResize(const u32 width, const u32 height) {
        mAspectRatio = CAST<f32>(width) / CAST<f32>(height);
        UpdateProjectionMatrix();
    }

    void Camera::Rotate(f32 deltaPitch, f32 deltaYaw) {
        mRotation.x += deltaPitch;
        mRotation.y += deltaYaw;

        constexpr f32 maxPitch = XM_PIDIV2 - 0.01f;  // clamp pitch to prevent flipping
        mRotation.x            = std::max(-maxPitch, std::min(maxPitch, mRotation.x));

        // Keep yaw in [0, 2PI] range
        if (mRotation.y > XM_2PI) {
            mRotation.y -= XM_2PI;
        } else if (mRotation.y < 0.0f) {
            mRotation.y += XM_2PI;
        }

        Matrix rotationMatrix = XMMatrixRotationRollPitchYaw(mRotation.x, mRotation.y, 0.0f);

        VectorSet defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        VectorSet defaultRight   = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        VectorSet defaultUp      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        mForward = XMVector3TransformCoord(defaultForward, rotationMatrix);
        mRight   = XMVector3TransformCoord(defaultRight, rotationMatrix);
        mUp      = XMVector3TransformCoord(defaultUp, rotationMatrix);

        // Update look-at point
        mAt = XMVectorAdd(mPosition, mForward);

        UpdateViewMatrix();
    }

    void Camera::MoveForward(f32 distance) {
        mPosition = XMVectorAdd(mPosition, XMVectorScale(mForward, distance));
        mAt       = XMVectorAdd(mPosition, mForward);
        UpdateViewMatrix();
    }

    void Camera::MoveRight(f32 distance) {
        mPosition = XMVectorAdd(mPosition, XMVectorScale(mRight, distance));
        mAt       = XMVectorAdd(mPosition, mForward);
        UpdateViewMatrix();
    }

    void Camera::SetPosition(const VectorSet& position) {
        mPosition = position;
        UpdateViewMatrix();
    }

    void Camera::SetFOV(const f32 fovY) {
        mFovY = XMConvertToRadians(fovY);
        UpdateProjectionMatrix();
    }

    void Camera::SetAspectRatio(const f32 ratio) {
        mAspectRatio = ratio;
        UpdateProjectionMatrix();
    }

    void Camera::SetClipPlanes(const f32 nearZ, const f32 farZ) {
        mNearZ = nearZ;
        mFarZ  = farZ;
        UpdateProjectionMatrix();
    }

    Matrix Camera::GetViewMatrix() const {
        return mViewMatrix;
    }

    Matrix Camera::GetProjectionMatrix() const {
        return mProjectionMatrix;
    }

    Matrix Camera::GetViewProjectionMatrix() const {
        return XMMatrixMultiply(mViewMatrix, mProjectionMatrix);
    }

    Float3 Camera::GetPosition() const {
        Float3 pos;
        XMStoreFloat3(&pos, mPosition);
        return pos;
    }

    void Camera::UpdateViewMatrix() {
        mViewMatrix = XMMatrixLookAtLH(mPosition, mAt, mUp);
    }

    void Camera::UpdateProjectionMatrix() {
        mProjectionMatrix = XMMatrixPerspectiveFovLH(mFovY, mAspectRatio, mNearZ, mFarZ);
    }
}  // namespace x