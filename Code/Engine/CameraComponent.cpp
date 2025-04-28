// Author: Jake Rieger
// Created: 3/25/2025.
//

#include "CameraComponent.hpp"

namespace x {
    CameraComponent::CameraComponent(const TransformComponent* transform) : mTransform(transform) {
        X_ASSERT((transform) && (mTransform));  // I'll need more robust checks but this will work for now
        UpdateVectors();
        RecalculateViewMatrix();
        RecalculateProjectionMatrix();
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
        return mTransform->GetPosition();
    }

    Float3 CameraComponent::GetRotation() const {
        return mTransform->GetRotation();
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

    const TransformComponent* CameraComponent::GetTransform() const {
        return mTransform;
    }

    Matrix CameraComponent::GetViewMatrix() const {
        return mViewMatrix;
    }

    Matrix CameraComponent::GetProjectionMatrix() const {
        return mProjectionMatrix;
    }

    Float3 CameraComponent::GetLookAtVector() const {
        return mLookAt;
    }

    Float3 CameraComponent::GetForwardVector() const {
        return mForward;
    }

    Float3 CameraComponent::GetRightVector() const {
        return mRight;
    }

    Float3 CameraComponent::GetUpVector() const {
        return mUp;
    }

    void CameraComponent::Update() {
        UpdateVectors();
        RecalculateViewMatrix();
    }

    void CameraComponent::OnResize(u32 width, u32 height) {
        mAspectRatio = (f32)width / (f32)height;
        RecalculateProjectionMatrix();
    }

    void CameraComponent::RecalculateViewMatrix() {
        const Float3 position = mTransform->GetPosition();
        if (position.x == mLookAt.x && position.y == mLookAt.y && position.z == mLookAt.z) { return; }
        mViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&mLookAt), XMLoadFloat3(&mUp));
    }

    void CameraComponent::RecalculateProjectionMatrix() {
        mProjectionMatrix = mOrthographic ? XMMatrixOrthographicLH(mWidth, mHeight, mNearZ, mFarZ)
                                          : XMMatrixPerspectiveFovLH(mFOV, mAspectRatio, mNearZ, mFarZ);
    }

    void CameraComponent::UpdateVectors() {
        if (!mTransform) {
            X_LOG_WARN("CameraComponent::UpdateVectors: Transform is null");
            return;
        }

        const Float3 rotation = mTransform->GetRotation();
        // const Matrix rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
        //                                                            XMConvertToDegrees(rotation.y),
        //                                                            XMConvertToDegrees(rotation.z));
        // TODO: Rotates extremely fast currently
        // This kind of fixes it but the rotation values are clearly not in degrees or radians. Rotation axes also
        // remain aligned when rotated instead of being relative to world(?) space. It works, but there's lots of
        // stuff to investigate here...
        const Matrix rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

        const VectorSet defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        const VectorSet defaultRight   = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        const VectorSet defaultUp      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        const auto forward = XMVector3TransformCoord(defaultForward, rotationMatrix);
        const auto right   = XMVector3TransformCoord(defaultRight, rotationMatrix);
        const auto up      = XMVector3TransformCoord(defaultUp, rotationMatrix);

        XMStoreFloat3(&mForward, forward);
        XMStoreFloat3(&mRight, right);
        XMStoreFloat3(&mUp, up);

        const Float3 position = mTransform->GetPosition();
        XMStoreFloat3(&mLookAt, XMVectorAdd(XMLoadFloat3(&position), XMLoadFloat3(&mForward)));
    }
}  // namespace x