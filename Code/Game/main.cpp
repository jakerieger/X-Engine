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

#include <Vendor/imgui/imgui.h>

using namespace x; // engine namespace

// TODO: Make this relative to the executable path, this is simply for testing (and because I'm lazy)
static str ContentPath(const str& filename) {
    const str root = R"(C:\Users\conta\Code\SpaceGame\Engine\Content\)";
    return root + filename;
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
        devConsole.RegisterCommand("r_ShowPostProcess",
                                   [this](auto args) {
                                       if (args.size() < 1) { return; }
                                       const auto show    = CAST<int>(strtol(args[0].c_str(), None, 10));
                                       _showPostProcessUI = show;
                                   });
        RasterizerStates::SetupRasterizerStates(_renderer);

        _monkeEntity         = state.CreateEntity();
        auto& monkeTransform = state.AddComponent<TransformComponent>(_monkeEntity);
        auto& monkeModel     = state.AddComponent<ModelComponent>(_monkeEntity);

        GenericLoader loader(_renderer);
        const auto monkeData = loader.LoadFromFile(ContentPath("Monke.glb"));

        TextureLoader texLoader(_renderer);
        const auto monkeAlbedo    = texLoader.LoadFromFile2D(ContentPath("Metal_Albedo.dds"));
        const auto monkeNormal    = texLoader.LoadFromFile2D(ContentPath("Metal_Normal.dds"));
        const auto monkeMetallic  = texLoader.LoadFromFile2D(ContentPath("Metal_Metallic.dds"));
        const auto monkeRoughness = texLoader.LoadFromFile2D(ContentPath("Metal_Roughness.dds"));

        _monkeMaterial = PBRMaterial::Create(_renderer);
        _monkeMaterial->SetTextureMaps(monkeAlbedo, monkeMetallic, monkeRoughness, monkeNormal);

        monkeModel.SetModelHandle(monkeData)
                  .SetMaterialHandle(_monkeMaterial)
                  .SetCastsShadows(true);

        const auto floorData = loader.LoadFromFile(ContentPath("Floor.glb"));

        const auto floorAlbedo = texLoader.LoadFromFile2D(ContentPath("checkerboard.dds"));
        const auto floorNormal = texLoader.LoadFromFile2D(ContentPath("Gold_Normal.dds"));

        _floorMaterial = PBRMaterial::Create(_renderer);
        _floorMaterial->SetTextureMaps(floorAlbedo, None, None, floorNormal);

        _floorEntity         = state.CreateEntity();
        auto& floorTransform = state.AddComponent<TransformComponent>(_floorEntity);
        auto& floorModel     = state.AddComponent<ModelComponent>(_floorEntity);

        floorTransform.SetPosition({0.0f, -1.0f, 0.0f});
        floorModel.SetModelHandle(floorData)
                  .SetMaterialHandle(_floorMaterial)
                  .SetCastsShadows(true);

        auto& camera = state.GetMainCamera();
        camera.SetFOV(70.0f);
        camera.SetPosition(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f));

        auto& sun     = state.GetLightState().Sun;
        sun.enabled   = true;
        sun.intensity = 2.0f;
        sun.color     = {1.0f, 1.0f, 1.0f, 1.0f};
        sun.direction = {-0.57f, 0.57f, 0.97f, 0.0f};

        _renderer.GetContext()->RSSetState(RasterizerStates::DefaultSolid.Get());

        // PostProcessSystem* postProcess = renderer.GetPostProcess();
        // _tonemap                       = postProcess->AddEffect<TonemapEffect>();
        // _tonemap->SetOperator(_tonemapOp);
        // _tonemap->SetExposure(_tonemapExposure);
        //
        // _colorGrade = postProcess->AddEffect<ColorGradeEffect>();
        // _colorGrade->SetContrast(_contrast);
        // _colorGrade->SetSaturation(_saturation);
        // _colorGrade->SetTemperature(_temperature);

        // _renderSystem->RegisterOccluders(_floorModel, _monkeModel);
        // _renderSystem->RegisterOpaqueObjects(_floorModel, _monkeModel);
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

    // game.EnableDebugUI();
    game.Run();

    return 0; // I know you don't have to, but I like the explicit nature of this.
}