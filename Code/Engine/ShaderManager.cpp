// Author: Jake Rieger
// Created: 3/1/2025.
//

#include "ShaderManager.hpp"

#pragma region Shaders
#include <OpaquePBR_PS.h>
#include <OpaquePBR_VS.h>
#include <ScreenTexture_VS.h>
#include <ScreenTexture_PS.h>
#include <ShadowPass_VS.h>
#include <ShadowPass_PS.h>
#include <Water_VS.h>
#include <Water_PS.h>
#pragma endregion

namespace x {
    shared_ptr<GraphicsShader> ShaderManager::GetGraphicsShader(u32 shaderId) {
        if (!mGraphicsShaders.contains(shaderId)) {
            X_LOG_ERROR("Graphics shader with id '%u' does not exist in cache", shaderId);
            return nullptr;
        }
        return mGraphicsShaders[shaderId];
    }

    shared_ptr<ComputeShader> ShaderManager::GetComputeShader(u32 shaderId) {
        if (!mComputeShaders.contains(shaderId)) {
            X_LOG_ERROR("Compute shader with id '%u' does not exist in cache", shaderId);
            return nullptr;
        }
        return mComputeShaders[shaderId];
    }

    bool ShaderManager::CreateGraphicsShader(
      RenderContext& context, u32 shaderId, const u8* vsCode, size_t vsSize, const u8* psCode, size_t psSize) {
        const auto shader     = make_shared<GraphicsShader>();
        shader->mVertexShader = make_unique<VertexShader>(context);
        shader->mPixelShader  = make_unique<PixelShader>(context);
        if (!shader->mVertexShader || !shader->mPixelShader) {
            X_LOG_ERROR("Failed to initialize shader")
            return false;
        }

        shader->mVertexShader->LoadFromMemory(vsCode, vsSize);
        shader->mPixelShader->LoadFromMemory(psCode, psSize);

        mGraphicsShaders[shaderId] = shader;

        return true;
    }

    bool ShaderManager::LoadShaders(RenderContext& context) {
        mGraphicsShaders.clear();
        mComputeShaders.clear();

        if (!CreateGraphicsShader(context,
                                  kOpaquePBRShaderId,
                                  X_ARRAY_W_SIZE(kOpaquePBR_VSBytes),
                                  X_ARRAY_W_SIZE(kOpaquePBR_PSBytes))) {
            X_LOG_ERROR("Failed to create OpaquePBR shader")
            return false;
        }
        X_LOG_INFO("Loaded OpaquePBR shader");

        if (!CreateGraphicsShader(context,
                                  kScreenQuadShaderId,
                                  X_ARRAY_W_SIZE(kScreenTexture_VSBytes),
                                  X_ARRAY_W_SIZE(kScreenTexture_PSBytes))) {
            X_LOG_ERROR("Failed to create ScreenQuad shader")
            return false;
        }
        X_LOG_INFO("Loaded ScreenQuad shader");

        if (!CreateGraphicsShader(context,
                                  kShadowMapShaderId,
                                  X_ARRAY_W_SIZE(kShadowPass_VSBytes),
                                  X_ARRAY_W_SIZE(kShadowPass_PSBytes))) {
            X_LOG_ERROR("Failed to create ShadowMap shader")
            return false;
        }
        X_LOG_INFO("Loaded ShadowMap shader");

        if (!CreateGraphicsShader(context,
                                  kWaterShaderId,
                                  X_ARRAY_W_SIZE(kWater_VSBytes),
                                  X_ARRAY_W_SIZE(kWater_PSBytes))) {
            X_LOG_ERROR("Failed to create Water shader")
            return false;
        }
        X_LOG_INFO("Loaded Water shader");

        X_LOG_INFO("All shaders loaded");
        return true;
    }
}  // namespace x