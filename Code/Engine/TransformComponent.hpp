// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Math.hpp"

namespace x {
    class TransformComponent {
    public:
        TransformComponent();
        void SetPosition(const Float3& position);
        void SetRotation(const Float3& rotation);
        void SetScale(const Float3& scale);

        Float3 GetPosition() const;
        Float3 GetRotation() const;
        Float3 GetScale() const;
        Matrix GetTransformMatrix() const;
        Matrix GetInverseTransformMatrix() const;

        void Translate(const Float3& translation);
        void Rotate(const Float3& rotation);
        void Scale(const Float3& scale);
        void Update();

    private:
        Float3 _position;
        Float3 _rotation;
        Float3 _scale;
        Matrix _transform;
        bool _needsUpdate;

        void UpdateTransformMatrix();
        static Matrix MatrixRotation(const Float3& eulerAngles);
        static Matrix MatrixTranslation(const Float3& translation);
        static Matrix MatrixScale(const Float3& scale);
    };
} // namespace x