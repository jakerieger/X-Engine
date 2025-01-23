// Author: Jake Rieger
// Created: 1/13/2025.
//

#include "TransformComponent.hpp"

namespace x {
    using namespace DirectX;

    TransformComponent::TransformComponent()
        : _position(0.0f, 0.0f, 0.0f), _rotation(0.0f, 0.0f, 0.0f), _scale(1.0f, 1.0f, 1.0f),
          _transform(XMMatrixIdentity()), _needsUpdate(true) {}

    void TransformComponent::SetPosition(const XMFLOAT3& position) {
        _position = position;
    }

    void TransformComponent::SetRotation(const XMFLOAT3& rotation) {
        _rotation = rotation;
    }

    void TransformComponent::SetScale(const XMFLOAT3& scale) {
        _scale = scale;
    }

    XMFLOAT3 TransformComponent::GetPosition() const {
        return _position;
    }

    XMFLOAT3 TransformComponent::GetRotation() const {
        return _rotation;
    }

    XMFLOAT3 TransformComponent::GetScale() const {
        return _scale;
    }

    XMMATRIX TransformComponent::GetTransformMatrix() const {
        return _transform;
    }

    XMMATRIX TransformComponent::GetInverseTransformMatrix() const {
        return XMMatrixInverse(None, _transform);
    }

    void TransformComponent::Translate(const XMFLOAT3& translation) {
        const XMVECTOR pos   = XMLoadFloat3(&_position);
        const XMVECTOR trans = XMLoadFloat3(&translation);
        XMStoreFloat3(&_position, XMVectorAdd(pos, trans));
        _needsUpdate = true;
    }

    void TransformComponent::Rotate(const XMFLOAT3& rotation) {
        const XMVECTOR currentRot = XMLoadFloat3(&_rotation);
        const XMVECTOR deltaRot   = XMLoadFloat3(&rotation);
        XMStoreFloat3(&_position, XMVectorAdd(currentRot, deltaRot));
        _needsUpdate = true;
    }

    void TransformComponent::Scale(const XMFLOAT3& scale) {
        const XMVECTOR currentScale = XMLoadFloat3(&_scale);
        const XMVECTOR deltaScale   = XMLoadFloat3(&scale);
        XMStoreFloat3(&_position, XMVectorAdd(currentScale, deltaScale));
        _needsUpdate = true;
    }

    void TransformComponent::Update() {
        if (_needsUpdate) UpdateTransformMatrix();
    }

    void TransformComponent::UpdateTransformMatrix() {
        const auto translation = MatrixTranslation(_position);
        const auto rotation    = MatrixRotation(_rotation);
        const auto scale       = MatrixScale(_scale);
        _transform             = scale * rotation * translation;
        _needsUpdate           = false;
    }

    XMMATRIX
    TransformComponent::MatrixRotation(const XMFLOAT3& eulerAngles) {
        const auto vecAngles     = XMLoadFloat3(&eulerAngles);
        const auto radiansAngles = XMVectorMultiply(vecAngles, XMVectorReplicate(XM_PI / 180.0f));
        return XMMatrixRotationRollPitchYawFromVector(radiansAngles);
    }

    XMMATRIX
    TransformComponent::MatrixTranslation(const XMFLOAT3& translation) {
        return XMMatrixTranslation(translation.x, translation.y, translation.z);
    }

    XMMATRIX TransformComponent::MatrixScale(const XMFLOAT3& scale) {
        return XMMatrixScaling(scale.x, scale.y, scale.z);
    }
}  // namespace x