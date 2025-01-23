// Author: Jake Rieger
// Created: 1/16/2025.
//

#pragma once

#include "Common/Types.hpp"
#include <DirectXMath.h>

namespace x {
    struct alignas(16) TransformMatrices {
        DirectX::XMMATRIX model;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX modelView;
        DirectX::XMMATRIX viewProjection;
        DirectX::XMMATRIX modelViewProjection;

        TransformMatrices(const DirectX::XMMATRIX& m, const DirectX::XMMATRIX& v, const DirectX::XMMATRIX& p) {
            this->model               = m;
            this->view                = v;
            this->projection          = p;
            this->modelView           = XMMatrixMultiply(m, v);
            this->viewProjection      = XMMatrixMultiply(v, p);
            this->modelViewProjection = XMMatrixMultiply(m, this->viewProjection);
        }
    };
}