#pragma once

#include "Renderer.hpp"
#include "D3D.hpp"
#include "Common/Types.hpp"

namespace x {
    struct alignas(16) SceneConstants {
        XMMATRIX viewProjection;
        XMFLOAT3 cameraPosition;
        f32 padding;
        XMFLOAT2 screenDimensions;
        f32 nearZ;
        f32 farZ;
    };

    struct alignas(16) LightData {
        static constexpr u32 kMaxLights = 1024;
        static constexpr u32 kTileSize  = 32;

        struct Light {
            XMFLOAT3 position;
            f32 range;
            XMFLOAT3 color;
            f32 intensity;
        };

        struct TileGrid {
            u32 lightCount;
            u32 lightIndices[256]; // max lights per tile
        };
    };

    // forward decl
    class Mesh;
    class Material;

    class RenderSystem {
    public:
        explicit RenderSystem(Renderer& renderer);

        // Prevent moves or copies
        RenderSystem(const RenderSystem& other)            = delete;
        RenderSystem(RenderSystem&& other)                 = delete;
        RenderSystem& operator=(const RenderSystem& other) = delete;
        RenderSystem& operator=(RenderSystem&& other)      = delete;

        void Initialize();
        void UpdateSceneConstants(const SceneConstants& constants);
        void CullLights(const vector<LightData::Light>& lights);

        void AddOpaqueMesh(const Mesh& mesh, const Material& material, const XMMATRIX& worldMatrix);
        void AddTransparentMesh(const Mesh& mesh, const Material& material, const XMMATRIX& worldMatrix);

        void Render();

    private:
        void CreateConstantBuffers();
        void CreateLightStructures();
        void SetupPipelines();

        Renderer& _renderer;
        ComPtr<ID3D11Buffer> _sceneConstantBuffer;
        ComPtr<ID3D11Buffer> _lightBuffer;
        ComPtr<ID3D11Buffer> _tileGrid;
        ComPtr<ID3D11ComputeShader> _lightCullCS;
        ComPtr<ID3D11DepthStencilState> _opaqueDepthState;
        ComPtr<ID3D11DepthStencilState> _transparentDepthState;
        ComPtr<ID3D11BlendState> _opaqueBlendState;
        ComPtr<ID3D11BlendState> _transparentBlendState;

        struct RenderItem {
            const Mesh* mesh;
            const Material* material;
            XMMATRIX worldMatrix;
        };

        vector<RenderItem> _opaqueQueue;
        vector<RenderItem> _transparentQueue;
    };
}