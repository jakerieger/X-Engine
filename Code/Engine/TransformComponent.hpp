// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "ComponentManager.hpp"
#include <DirectXMath.h>

namespace x {
    class TransformComponent {
    public:
        TransformComponent();
        void SetPosition(const DirectX::XMFLOAT3& position);
        void SetRotation(const DirectX::XMFLOAT3& rotation);
        void SetScale(const DirectX::XMFLOAT3& scale);

        DirectX::XMFLOAT3 GetPosition() const;
        DirectX::XMFLOAT3 GetRotation() const;
        DirectX::XMFLOAT3 GetScale() const;
        DirectX::XMMATRIX GetTransformMatrix() const;
        DirectX::XMMATRIX GetInverseTransformMatrix() const;

        void Translate(const DirectX::XMFLOAT3& translation);
        void Rotate(const DirectX::XMFLOAT3& rotation);
        void Scale(const DirectX::XMFLOAT3& scale);
        void Update();

    private:
        DirectX::XMFLOAT3 _position;
        DirectX::XMFLOAT3 _rotation;
        DirectX::XMFLOAT3 _scale;
        DirectX::XMMATRIX _transform;
        bool _needsUpdate;

        void UpdateTransformMatrix();
        static DirectX::XMMATRIX MatrixRotation(const DirectX::XMFLOAT3& eulerAngles);
        static DirectX::XMMATRIX MatrixTranslation(const DirectX::XMFLOAT3& translation);
        static DirectX::XMMATRIX MatrixScale(const DirectX::XMFLOAT3& scale);
    };
} // namespace x