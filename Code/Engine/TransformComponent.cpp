// Author: Jake Rieger
// Created: 1/13/2025.
//

#include "TransformComponent.hpp"
#include "Common/Typedefs.hpp"

namespace x {
    TransformComponent::TransformComponent()
        : mPosition(0.0f, 0.0f, 0.0f), mRotation(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f),
          mTransform(XMMatrixIdentity()), mNeedsUpdate(true) {}

    void TransformComponent::SetPosition(const Float3& position) {
        mPosition    = position;
        mNeedsUpdate = true;
    }

    void TransformComponent::SetRotation(const Float3& rotation) {
        mRotation    = rotation;
        mNeedsUpdate = true;
    }

    void TransformComponent::SetScale(const Float3& scale) {
        mScale       = scale;
        mNeedsUpdate = true;
    }

    Float3 TransformComponent::GetPosition() const {
        return mPosition;
    }

    Float3 TransformComponent::GetRotation() const {
        return mRotation;
    }

    Float3 TransformComponent::GetScale() const {
        return mScale;
    }

    Matrix TransformComponent::GetTransformMatrix() const {
        return mTransform;
    }

    Matrix TransformComponent::GetInverseTransformMatrix() const {
        return XMMatrixInverse(nullptr, mTransform);
    }

    void TransformComponent::Translate(const Float3& translation) {
        const XMVECTOR pos   = XMLoadFloat3(&mPosition);
        const XMVECTOR trans = XMLoadFloat3(&translation);
        XMStoreFloat3(&mPosition, XMVectorAdd(pos, trans));
        mNeedsUpdate = true;
    }

    void TransformComponent::Rotate(const Float3& rotation) {
        const XMVECTOR currentRot = XMLoadFloat3(&mRotation);
        const XMVECTOR deltaRot   = XMLoadFloat3(&rotation);
        XMStoreFloat3(&mPosition, XMVectorAdd(currentRot, deltaRot));
        mNeedsUpdate = true;
    }

    void TransformComponent::Scale(const Float3& scale) {
        const XMVECTOR currentScale = XMLoadFloat3(&mScale);
        const XMVECTOR deltaScale   = XMLoadFloat3(&scale);
        XMStoreFloat3(&mPosition, XMVectorAdd(currentScale, deltaScale));
        mNeedsUpdate = true;
    }

    void TransformComponent::Update() {
        if (mNeedsUpdate) UpdateTransformMatrix();
    }

    void TransformComponent::UpdateTransformMatrix() {
        const auto translation = MatrixTranslation(mPosition);
        const auto rotation    = MatrixRotation(mRotation);
        const auto scale       = MatrixScale(mScale);
        mTransform             = scale * rotation * translation;
        mNeedsUpdate           = false;
    }

    Matrix TransformComponent::MatrixRotation(const Float3& eulerAngles) {
        const auto vecAngles     = XMLoadFloat3(&eulerAngles);
        const auto radiansAngles = XMVectorMultiply(vecAngles, XMVectorReplicate(XM_PI / 180.0f));
        return XMMatrixRotationRollPitchYawFromVector(radiansAngles);
    }

    Matrix TransformComponent::MatrixTranslation(const Float3& translation) {
        return XMMatrixTranslation(translation.x, translation.y, translation.z);
    }

    Matrix TransformComponent::MatrixScale(const Float3& scale) {
        return XMMatrixScaling(scale.x, scale.y, scale.z);
    }
}  // namespace x