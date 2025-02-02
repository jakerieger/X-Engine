#include "TonemapEffect.hpp"

namespace x {
    bool TonemapEffect::Initialize() {
        _computeShader.LoadFromFile(R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\Tonemap.hlsl)");
        return CreateResources();
    }

    bool TonemapEffect::CreateResources() {
        return IComputeEffect::CreateResources();
    }

    void TonemapEffect::UpdateConstants() {
        IComputeEffect::UpdateConstants();
    }
}