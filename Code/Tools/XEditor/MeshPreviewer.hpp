// Author: Jake Rieger
// Created: 3/18/2025.
//

#pragma once

#include <imgui.h>

#include "Engine/ModelComponent.hpp"
#include "Engine/RenderContext.hpp"
#include "Engine/Viewport.hpp"
#include "Engine/BasicLitMaterial.hpp"

namespace x {
    class MeshPreviewer {
    public:
        explicit MeshPreviewer(RenderContext& context) : mContext(context), mViewport(context) {
            mViewport.SetClearColor(Colors::DarkGrey);
            mLightState.mSun.mEnabled   = true;
            mLightState.mSun.mDirection = Float4(0.5f, 0.5f, -0.5f, 1.f);
            mCamera.SetPosition(XMVectorSet(0, 2, -6, 0));
        }

        void SetModel(unique_ptr<ModelComponent> model) {
            if (mMaterial.get() == nullptr) { mMaterial = make_shared<BasicLitMaterial>(mContext); }
            mCurrentModel = std::move(model);
            mCurrentModel->SetMaterial(mMaterial);
            mCurrentModel->SetMaterialId(1);
            mCurrentModel->SetModelId(1);
        }

        ID3D11ShaderResourceView* Render(ImVec2 viewportSize) {
            if (viewportSize.x <= 0 || viewportSize.y <= 0) { return nullptr; }

            mCamera.SetAspectRatio(viewportSize.x / viewportSize.y);
            const auto transforms =
              TransformMatrices(XMMatrixIdentity(), mCamera.GetViewMatrix(), mCamera.GetProjectionMatrix());

            mViewport.Resize((u32)viewportSize.x, (u32)viewportSize.y);
            mViewport.ClearAll();
            mViewport.BindRenderTarget();
            mViewport.AttachViewport();

            if (mCurrentModel.get() != nullptr) {
                mCurrentModel->Draw(mContext, transforms, mLightState, mCamera.GetPosition());
            }

            return mViewport.GetShaderResourceView().Get();
        }

    private:
        RenderContext& mContext;
        Viewport mViewport;
        Camera mCamera;
        LightState mLightState;
        unique_ptr<ModelComponent> mCurrentModel {nullptr};
        shared_ptr<BasicLitMaterial> mMaterial;
    };
}  // namespace x
