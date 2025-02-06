// Author: Jake Rieger
// Created: 1/13/2025.
//

#include "TransformComponent.hpp"
#include "Common/Types.hpp"

namespace x {
    TransformComponent::TransformComponent()
        : _position(0.0f, 0.0f, 0.0f), _rotation(0.0f, 0.0f, 0.0f), _scale(1.0f, 1.0f, 1.0f),
          _transform(XMMatrixIdentity()), _needsUpdate(true) {}

    void TransformComponent::SetPosition(const Float3& position) {
        _position    = position;
        _needsUpdate = true;
    }

    void TransformComponent::SetRotation(const Float3& rotation) {
        _rotation    = rotation;
        _needsUpdate = true;
    }

    void TransformComponent::SetScale(const Float3& scale) {
        _scale       = scale;
        _needsUpdate = true;
    }

    Float3 TransformComponent::GetPosition() const {
        return _position;
    }

    Float3 TransformComponent::GetRotation() const {
        return _rotation;
    }

    Float3 TransformComponent::GetScale() const {
        return _scale;
    }

    Matrix TransformComponent::GetTransformMatrix() const {
        return _transform;
    }

    Matrix TransformComponent::GetInverseTransformMatrix() const {
        return XMMatrixInverse(None, _transform);
    }

    void TransformComponent::Translate(const Float3& translation) {
        const XMVECTOR pos   = XMLoadFloat3(&_position);
        const XMVECTOR trans = XMLoadFloat3(&translation);
        XMStoreFloat3(&_position, XMVectorAdd(pos, trans));
        _needsUpdate = true;
    }

    void TransformComponent::Rotate(const Float3& rotation) {
        const XMVECTOR currentRot = XMLoadFloat3(&_rotation);
        const XMVECTOR deltaRot   = XMLoadFloat3(&rotation);
        XMStoreFloat3(&_position, XMVectorAdd(currentRot, deltaRot));
        _needsUpdate = true;
    }

    void TransformComponent::Scale(const Float3& scale) {
        const XMVECTOR currentScale = XMLoadFloat3(&_scale);
        const XMVECTOR deltaScale   = XMLoadFloat3(&scale);
        XMStoreFloat3(&_position, XMVectorAdd(currentScale, deltaScale));
        _needsUpdate = true;
    }

    void TransformComponent::Update() {
        if (_needsUpdate)
            UpdateTransformMatrix();
    }

    void TransformComponent::UpdateTransformMatrix() {
        const auto translation = MatrixTranslation(_position);
        const auto rotation    = MatrixRotation(_rotation);
        const auto scale       = MatrixScale(_scale);
        _transform             = scale * rotation * translation;
        _needsUpdate           = false;
    }

    Matrix
    TransformComponent::MatrixRotation(const Float3& eulerAngles) {
        const auto vecAngles     = XMLoadFloat3(&eulerAngles);
        const auto radiansAngles = XMVectorMultiply(vecAngles, XMVectorReplicate(XM_PI / 180.0f));
        return XMMatrixRotationRollPitchYawFromVector(radiansAngles);
    }

    Matrix
    TransformComponent::MatrixTranslation(const Float3& translation) {
        return XMMatrixTranslation(translation.x, translation.y, translation.z);
    }

    Matrix TransformComponent::MatrixScale(const Float3& scale) {
        return XMMatrixScaling(scale.x, scale.y, scale.z);
    }
} // namespace x