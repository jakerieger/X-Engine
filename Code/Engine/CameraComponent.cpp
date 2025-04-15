// Author: Jake Rieger
// Created: 3/25/2025.
//

#include "CameraComponent.hpp"

namespace x {
    CameraComponent::CameraComponent() {
        RecalculateViewMatrix();
        RecalculateProjectionMatrix();
    }

    CameraComponent& CameraComponent::SetPosition(const Float3& position) {
        mPosition = position;
        RecalculateViewMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetFOVDegrees(const f32 deg) {
        mFOV = XMConvertToRadians(deg);
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetFOVRadians(const f32 rad) {
        mFOV = rad;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetAspectRatio(const f32 aspect) {
        mAspectRatio = aspect;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetNearPlane(const f32 nearZ) {
        mNearZ = nearZ;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetFarPlane(const f32 farZ) {
        mFarZ = farZ;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetClipPlanes(const f32 nearZ, const f32 farZ) {
        mNearZ = nearZ;
        mFarZ  = farZ;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetOrthographic(const bool ortho) {
        mOrthographic = ortho;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetWidth(const f32 width) {
        if (width < 1.0f) { return *this; }
        mWidth = width;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetHeight(const f32 height) {
        if (height < 1.0f) { return *this; }
        mHeight = height;
        RecalculateProjectionMatrix();
        return *this;
    }

    CameraComponent& CameraComponent::SetWidthHeight(const f32 width, const f32 height) {
        if (width < 1.0f || height < 1.0f) { return *this; }
        mWidth  = width;
        mHeight = height;
        RecalculateProjectionMatrix();
        return *this;
    }

    Float3 CameraComponent::GetPosition() const {
        return mPosition;
    }

    f32 CameraComponent::GetFOVRadians() const {
        return mFOV;
    }

    f32 CameraComponent::GetFOVDegrees() const {
        return XMConvertToDegrees(mFOV);
    }

    f32 CameraComponent::GetAspectRatio() const {
        return mAspectRatio;
    }

    f32 CameraComponent::GetNearPlane() const {
        return mNearZ;
    }

    f32 CameraComponent::GetFarPlane() const {
        return mFarZ;
    }

    void CameraComponent::GetClipPlanes(f32& nearZ, f32& farZ) const {
        nearZ = mNearZ;
        farZ  = mFarZ;
    }

    bool CameraComponent::GetOrthographic() const {
        return mOrthographic;
    }

    f32 CameraComponent::GetWidth() const {
        return mWidth;
    }

    f32 CameraComponent::GetHeight() const {
        return mHeight;
    }

    void CameraComponent::GetWidthHeight(f32& width, f32& height) const {
        width  = mWidth;
        height = mHeight;
    }

    Matrix CameraComponent::GetViewMatrix() const {
        return mViewMatrix;
    }

    Matrix CameraComponent::GetProjectionMatrix() const {
        return mProjectionMatrix;
    }

    void CameraComponent::Rotate(f32 deltaPitch, f32 deltaYaw) {
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

        const Matrix rotationMatrix    = XMMatrixRotationRollPitchYaw(mRotation.x, mRotation.y, 0.0f);
        const VectorSet defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        const VectorSet defaultRight   = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        const VectorSet defaultUp      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        const auto forward = XMVector3TransformCoord(defaultForward, rotationMatrix);
        const auto right   = XMVector3TransformCoord(defaultRight, rotationMatrix);
        const auto up      = XMVector3TransformCoord(defaultUp, rotationMatrix);

        XMStoreFloat3(&mForward, forward);
        XMStoreFloat3(&mRight, right);
        XMStoreFloat3(&mUp, up);
        XMStoreFloat3(&mLookAt, XMVectorAdd(XMLoadFloat3(&mPosition), forward));

        RecalculateViewMatrix();
    }

    void CameraComponent::MoveForward(f32 distance) {
        XMStoreFloat3(&mPosition, XMVectorScale(XMLoadFloat3(&mForward), distance));
        XMStoreFloat3(&mLookAt, XMVectorAdd(XMLoadFloat3(&mPosition), XMLoadFloat3(&mForward)));
        RecalculateViewMatrix();
    }

    void CameraComponent::MoveRight(f32 distance) {
        XMStoreFloat3(&mPosition,
                      XMVectorAdd(XMLoadFloat3(&mPosition), XMVectorScale(XMLoadFloat3(&mRight), distance)));
        XMStoreFloat3(&mLookAt, XMVectorAdd(XMLoadFloat3(&mPosition), XMLoadFloat3(&mForward)));
        RecalculateViewMatrix();
    }

    void CameraComponent::OnResize(u32 width, u32 height) {
        mAspectRatio = (f32)width / (f32)height;
        RecalculateProjectionMatrix();
    }

    void CameraComponent::RecalculateViewMatrix() {
        mViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mLookAt), XMLoadFloat3(&mUp));
    }

    void CameraComponent::RecalculateProjectionMatrix() {
        mProjectionMatrix = mOrthographic ? XMMatrixOrthographicLH(mWidth, mHeight, mNearZ, mFarZ)
                                          : XMMatrixPerspectiveFovLH(mFOV, mAspectRatio, mNearZ, mFarZ);
    }
}  // namespace x