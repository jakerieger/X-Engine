// Author: Jake Rieger
// Created: 1/16/2025.
//

#pragma once

#include "Math.hpp"

namespace x {
    struct alignas(16) TransformMatrices {
        Matrix model;
        Matrix view;
        Matrix projection;
        Matrix modelView;
        Matrix viewProjection;
        Matrix modelViewProjection;

        TransformMatrices(const Matrix& m, const Matrix& v, const Matrix& p) {
            this->model      = XMMatrixTranspose(m);
            this->view       = XMMatrixTranspose(v);
            this->projection = XMMatrixTranspose(p);

            const auto mv  = XMMatrixMultiply(m, v);
            const auto vp  = XMMatrixMultiply(v, p);
            const auto mvp = XMMatrixMultiply(m, vp);

            this->modelView           = XMMatrixTranspose(mv);
            this->viewProjection      = XMMatrixTranspose(vp);
            this->modelViewProjection = XMMatrixTranspose(mvp);
        }
    };
}  // namespace x