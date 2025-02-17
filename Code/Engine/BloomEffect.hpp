#pragma once

#include "ComputeEffect.hpp"

namespace x {
    static constexpr DXGI_FORMAT kBloomFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

    #pragma region Subpasses
    class BloomSubpassBrightness final : public IComputeEffect {
    public:
        explicit BloomSubpassBrightness(RenderContext& context) : IComputeEffect(context) {
            _format = kBloomFormat;
        }

        bool Initialize() override;
    };

    class BloomSubpassBlurHorizontal final : public IComputeEffect {
    public:
        explicit BloomSubpassBlurHorizontal(RenderContext& context) : IComputeEffect(context) {
            _format = kBloomFormat;
        }

        bool Initialize() override;
    };

    class BloomSubpassBlurVertical final : public IComputeEffect {
    public:
        explicit BloomSubpassBlurVertical(RenderContext& context) : IComputeEffect(context) {
            _format = kBloomFormat;
        }

        bool Initialize() override;
    };
    #pragma endregion

    class BloomEffect final : public IComputeEffect {
    public:
        explicit BloomEffect(RenderContext& context) : IComputeEffect(context), _brightnessPass(context),
                                                       _blurHorizontalPass(context), _blurVerticalPass(context) {
            _format = kBloomFormat;
        }

        bool Initialize() override {
            const auto brightnessSuccess = _brightnessPass.Initialize();
            const auto blurHSuccess      = _blurHorizontalPass.Initialize();
            const auto blurVSuccess      = _blurVerticalPass.Initialize();

            return brightnessSuccess && blurHSuccess && blurVSuccess;
        }

        void Execute(ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output) override;

    private:
        BloomSubpassBrightness _brightnessPass;
        BloomSubpassBlurHorizontal _blurHorizontalPass;
        BloomSubpassBlurVertical _blurVerticalPass;
    };
}