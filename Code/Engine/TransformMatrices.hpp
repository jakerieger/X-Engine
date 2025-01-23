// Author: Jake Rieger
// Created: 1/16/2025.
//

#pragma once

#include "Common/Types.hpp"
#include <DirectXMath.h>

namespace x {
    struct TransformMatrices {
        DirectX::XMMATRIX model;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
        DirectX::XMMATRIX modelView;
        DirectX::XMMATRIX viewProjection;
        DirectX::XMMATRIX modelViewProjection;

        TransformMatrices(DirectX::XMMATRIX m, DirectX::XMMATRIX v, DirectX::XMMATRIX p) {
            this->model               = m;
            this->view                = v;
            this->projection          = p;
            this->modelView           = DirectX::XMMatrixMultiply(m, v);
            this->viewProjection      = DirectX::XMMatrixMultiply(v, p);
            this->modelViewProjection = DirectX::XMMatrixMultiply(m, this->viewProjection);
        }
    };
}