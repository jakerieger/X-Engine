#include "Engine/Game.hpp"
#include "Engine/Model.hpp"
#include "Engine/GenericLoader.hpp"
#include "Engine/Material.hpp"
#include "Engine/RasterizerState.hpp"
#include "Engine/Texture.hpp"
#include "Common/Str.hpp"
#include "Common/Timer.hpp"
#include "Engine/TonemapEffect.hpp"
#include "Engine/ColorGradeEffect.hpp"
#include "Common/Filesystem.hpp"

#include <Vendor/imgui/imgui.h>

using namespace x; // engine namespace
using namespace x::Filesystem;

// TODO: Make this relative to the executable path, this is simply for testing (and because I'm lazy)
static str ContentPath(const str& filename) {
    auto filePath    = Path(__FILE__).Parent();
    auto contentPath = filePath / ".." / ".." / "Engine" / "Content";
    return (contentPath / filename).Str();
}

class SpaceGame final : public IGame {
    shared_ptr<PBRMaterial> _monkeMaterial;
    shared_ptr<PBRMaterial> _floorMaterial;

    TonemapEffect* _tonemap       = None;
    ColorGradeEffect* _colorGrade = None;

    f32 _contrast              = 1.0f;
    f32 _saturation            = 1.0f;
    f32 _temperature           = 6500.0f;
    TonemapOperator _tonemapOp = TonemapOperator::ACES;
    f32 _tonemapExposure       = 1.0f;
    bool _showPostProcessUI    = false;

    EntityId _monkeEntity;
    EntityId _floorEntity;

public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {}

    void LoadContent(GameState& state) override {
        _devConsole.RegisterCommand("r_ShowPostProcess",
                                    [this](auto args) {
                                        if (args.size() < 1) { return; }
                                        const auto show    = CAST<int>(strtol(args[0].c_str(), None, 10));
                                        _showPostProcessUI = show;
                                    });
        RasterizerStates::SetupRasterizerStates(_renderContext); // Setup our rasterizer states for future use

        GenericLoader loader(_renderContext);
        TextureLoader texLoader(_renderContext);

        const auto floorData = loader.LoadFromFile(ContentPath("Floor.glb"));

        const auto floorAlbedo = texLoader.LoadFromFile2D(ContentPath("checkerboard.dds"));
        const auto floorNormal = texLoader.LoadFromFile2D(ContentPath("Gold_Normal.dds"));

        _floorMaterial = PBRMaterial::Create(_renderContext);
        _floorMaterial->SetTextureMaps(floorAlbedo, None, None, floorNormal);

        _floorEntity         = state.CreateEntity();
        auto& floorTransform = state.AddComponent<TransformComponent>(_floorEntity);
        auto& floorModel     = state.AddComponent<ModelComponent>(_floorEntity);

        floorTransform.SetPosition({0.0f, -1.0f, 0.0f});
        floorModel.SetModelHandle(floorData)
                  .SetMaterialHandle(_floorMaterial)
                  .SetCastsShadows(true);

        _monkeEntity         = state.CreateEntity();
        auto& monkeTransform = state.AddComponent<TransformComponent>(_monkeEntity);
        auto& monkeModel     = state.AddComponent<ModelComponent>(_monkeEntity);

        const auto monkeData = loader.LoadFromFile(ContentPath("Monke.glb"));

        const auto monkeAlbedo    = texLoader.LoadFromFile2D(ContentPath("Metal_Albedo.dds"));
        const auto monkeNormal    = texLoader.LoadFromFile2D(ContentPath("Metal_Normal.dds"));
        const auto monkeMetallic  = texLoader.LoadFromFile2D(ContentPath("Metal_Metallic.dds"));
        const auto monkeRoughness = texLoader.LoadFromFile2D(ContentPath("Metal_Roughness.dds"));

        _monkeMaterial = PBRMaterial::Create(_renderContext);
        _monkeMaterial->SetTextureMaps(monkeAlbedo, monkeMetallic, monkeRoughness, monkeNormal);

        monkeModel.SetModelHandle(monkeData).SetMaterialHandle(_monkeMaterial).SetCastsShadows(true);

        auto& camera = state.GetMainCamera();
        camera.SetFOV(70.0f);
        camera.SetPosition(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f));

        auto& sun       = state.GetLightState().Sun;
        sun.enabled     = true;
        sun.intensity   = 2.0f;
        sun.color       = {1.0f, 1.0f, 1.0f, 1.0f};
        sun.direction   = {0.57f, 0.57f, -0.57f, 0.0f};
        sun.castsShadow = true;

        // Calculate our light view projection for shadow mapping
        // viewWidth was set by trial and error, 10.0 just looked the best to me
        const auto lvp    = CalculateLightViewProjection(sun, 10.0f, state.GetMainCamera().GetAspectRatio());
        sun.lightViewProj = XMMatrixTranspose(lvp);
        // Functions that return matrices will never transpose them (for consistency-sake)

        auto& pointLight0     = state.GetLightState().PointLights[0];
        pointLight0.enabled   = true;
        pointLight0.intensity = 20.0f;
        pointLight0.color     = {1.0f, 0.0f, 0.0f};
        pointLight0.position  = {5.0f, 3.0f, 0.0f};

        auto& pointLight1     = state.GetLightState().PointLights[1];
        pointLight1.enabled   = false;
        pointLight1.intensity = 20.0f;
        pointLight1.color     = {0.0f, 1.0f, 0.0f};
        pointLight1.position  = {-5.0f, 3.0f, 0.0f};

        auto& pointLight2     = state.GetLightState().PointLights[2];
        pointLight2.enabled   = false;
        pointLight2.intensity = 20.0f;
        pointLight2.color     = {0.0f, 0.0f, 1.0f};
        pointLight2.position  = {0.0f, 3.0f, 0.0f};

        auto& areaLight0      = state.GetLightState().AreaLights[0];
        areaLight0.enabled    = true;
        areaLight0.intensity  = 1.0f;
        areaLight0.color      = {1.0f, 0.0f, 1.0f};
        areaLight0.dimensions = {10.f, 10.f};
        areaLight0.position   = {0.0f, 0.0f, -5.0f};
        areaLight0.direction  = {0.0f, 0.0f, 5.0f};

        _renderContext.GetDeviceContext()->RSSetState(RasterizerStates::DefaultSolid.Get());
    }

    void UnloadContent() override {}

    void Update(GameState& state, const Clock& clock) override {
        auto view = state.GetMainCamera().GetViewMatrix();
        auto proj = state.GetMainCamera().GetProjectionMatrix();

        // Update transform components
        std::unordered_map<EntityId, TransformComponent*> entitiesWithTransform;
        for (auto [entity, transform] : state.GetComponents<TransformComponent>().GetMutable()) {
            transform.Update();
            entitiesWithTransform[entity] = &transform;
        }

        // Iterate entities and update model material params
        for (auto [entity, model] : state.GetComponents<ModelComponent>().GetMutable()) {
            TransformComponent* transform = None;
            if (entitiesWithTransform.contains(entity)) { transform = entitiesWithTransform[entity]; }

            if (transform) {
                model.UpdateMaterialParams({transform->GetTransformMatrix(), view, proj},
                                           state.GetLightState(),
                                           state.GetMainCamera().GetPosition());
            } else {
                model.UpdateMaterialParams({XMMatrixIdentity(), view, proj},
                                           state.GetLightState(),
                                           state.GetMainCamera().GetPosition());
            }
        }
    }

    void DrawDebugUI() override {
        static constexpr std::array<const char*, 4> tonemapOpNames = {"ACES", "Reinhard", "Filmic", "Linear"};
        static bool dropdownValueChanged                           = false;

        if (_showPostProcessUI) {
            // ImGui::Begin("Post Processing");
            //
            // ImGui::SliderFloat("Contrast", &_contrast, 0.0f, 2.0f);
            // ImGui::SliderFloat("Saturation", &_saturation, 0.0f, 2.0f);
            // ImGui::SliderFloat("Temperature", &_temperature, 1000.0f, 10000.0f);
            // ImGui::Separator();
            // ImGui::SliderFloat("Exposure", &_tonemapExposure, 0.0f, 2.0f);
            //
            // if (ImGui::BeginCombo("Tonemap Operator", tonemapOpNames[CAST<u32>(_tonemapOp)])) {
            //     for (size_t i = 0; i < tonemapOpNames.size(); i++) {
            //         const auto opName     = tonemapOpNames[i];
            //         const auto currentOp  = CAST<TonemapOperator>(i);
            //         const bool isSelected = (_tonemapOp == currentOp);
            //
            //         if (ImGui::Selectable(opName, isSelected)) {
            //             _tonemapOp           = currentOp;
            //             dropdownValueChanged = true;
            //         }
            //
            //         if (isSelected) { ImGui::SetItemDefaultFocus(); }
            //     }
            //     ImGui::EndCombo();
            // }
            //
            // ImGui::End();
            //
            // _colorGrade->SetContrast(_contrast);
            // _colorGrade->SetSaturation(_saturation);
            // _colorGrade->SetTemperature(_temperature);
            // _tonemap->SetExposure(_tonemapExposure);
            // _tonemap->SetOperator(_tonemapOp);
        }
    }

    void OnResize(u32 width, u32 height) override {}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    SpaceGame game(hInstance);

    #ifndef NDEBUG
    game.EnableConsole();
    #endif

    game.EnableDebugUI();
    game.Run();

    return 0; // I know you don't have to, but I like the explicit nature of this.
}