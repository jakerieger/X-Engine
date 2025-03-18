// Author: Jake Rieger
// Created: 3/1/2025.
//

#pragma once

#include "EngineCommon.hpp"
#include "Shader.hpp"

namespace x {
    static constexpr u32 kOpaquePBRShaderId      = 0;
    static constexpr u32 kTransparentPBRShaderId = 1;
    static constexpr u32 kScreenQuadShaderId     = 2;
    static constexpr u32 kShadowMapShaderId      = 3;
    static constexpr u32 kWaterShaderId          = 4;
    static constexpr u32 kBasicLitShaderId       = 5;

    class ShaderManager {
        friend class Game;
        X_CLASS_PREVENT_MOVES_COPIES(ShaderManager);

        ShaderManager() = default;

    public:
        static shared_ptr<GraphicsShader> GetGraphicsShader(u32 shaderId);
        static shared_ptr<ComputeShader> GetComputeShader(u32 shaderId);

    private:
        static bool LoadShaders(RenderContext& context);
        static bool CreateGraphicsShader(
          RenderContext& context, u32 shaderId, const u8* vsCode, size_t vsSize, const u8* psCode, size_t psSize);

        // Shader cache
        inline static unordered_map<u32, shared_ptr<GraphicsShader>> mGraphicsShaders;
        inline static unordered_map<u32, shared_ptr<ComputeShader>> mComputeShaders;
    };
}  // namespace x
