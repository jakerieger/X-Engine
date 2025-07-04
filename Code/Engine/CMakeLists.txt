project(XENGINE)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_library(x STATIC
    ${IMGUI_SOURCES}
    ${STB_SOURCES}
    ${COMMON_SOURCES}
    # Engine sources
    ${ENGINE_DIR}/ArenaAllocator.cpp
    ${ENGINE_DIR}/ArenaAllocator.hpp
    ${ENGINE_DIR}/AssetManager.cpp
    ${ENGINE_DIR}/AssetManager.hpp
    ${ENGINE_DIR}/BasicLitMaterial.cpp
    ${ENGINE_DIR}/BasicLitMaterial.hpp
    ${ENGINE_DIR}/BehaviorComponent.cpp
    ${ENGINE_DIR}/BehaviorComponent.hpp
    ${ENGINE_DIR}/BloomEffect.cpp
    ${ENGINE_DIR}/BloomEffect.hpp
    ${ENGINE_DIR}/Camera.cpp
    ${ENGINE_DIR}/Camera.hpp
    ${ENGINE_DIR}/CameraComponent.cpp
    ${ENGINE_DIR}/CameraComponent.hpp
    ${ENGINE_DIR}/Clock.cpp
    ${ENGINE_DIR}/Clock.hpp
    ${ENGINE_DIR}/Color.cpp
    ${ENGINE_DIR}/Color.hpp
    ${ENGINE_DIR}/ColorGradeEffect.cpp
    ${ENGINE_DIR}/ColorGradeEffect.hpp
    ${ENGINE_DIR}/ComponentManager.hpp
    ${ENGINE_DIR}/ComputeEffect.hpp
    ${ENGINE_DIR}/D3D.hpp
    ${ENGINE_DIR}/DebugUI.hpp
    ${ENGINE_DIR}/DeferredActionQueue.hpp
    ${ENGINE_DIR}/DevConsole.cpp
    ${ENGINE_DIR}/DevConsole.hpp
    ${ENGINE_DIR}/EngineCommon.hpp
    ${ENGINE_DIR}/EntityId.hpp
    ${ENGINE_DIR}/Event.hpp
    ${ENGINE_DIR}/EventEmitter.hpp
    ${ENGINE_DIR}/EventListener.hpp
    ${ENGINE_DIR}/Game.cpp
    ${ENGINE_DIR}/Game.hpp
    ${ENGINE_DIR}/GeometryBuffer.hpp
    ${ENGINE_DIR}/Input.hpp
    ${ENGINE_DIR}/InputCodes.hpp
    ${ENGINE_DIR}/InputLayouts.hpp
    ${ENGINE_DIR}/LightPass.cpp
    ${ENGINE_DIR}/LightPass.hpp
    ${ENGINE_DIR}/Lights.hpp
    ${ENGINE_DIR}/Material.hpp
    ${ENGINE_DIR}/MaterialParser.cpp
    ${ENGINE_DIR}/MaterialParser.hpp
    ${ENGINE_DIR}/Math.hpp
    ${ENGINE_DIR}/Model.cpp
    ${ENGINE_DIR}/Model.hpp
    ${ENGINE_DIR}/ModelComponent.cpp
    ${ENGINE_DIR}/ModelComponent.hpp
    ${ENGINE_DIR}/ModelLoader.cpp
    ${ENGINE_DIR}/ModelLoader.hpp
    ${ENGINE_DIR}/Mouse.cpp
    ${ENGINE_DIR}/Mouse.hpp
    ${ENGINE_DIR}/PBRMaterial.cpp
    ${ENGINE_DIR}/PBRMaterial.hpp
    ${ENGINE_DIR}/PoolAllocator.cpp
    ${ENGINE_DIR}/PoolAllocator.hpp
    ${ENGINE_DIR}/PostProcessPass.cpp
    ${ENGINE_DIR}/PostProcessPass.hpp
    ${ENGINE_DIR}/RasterizerState.cpp
    ${ENGINE_DIR}/RasterizerState.hpp
    ${ENGINE_DIR}/RenderContext.cpp
    ${ENGINE_DIR}/RenderContext.hpp
    ${ENGINE_DIR}/RenderSystem.cpp
    ${ENGINE_DIR}/RenderSystem.hpp
    ${ENGINE_DIR}/Resource.hpp
    ${ENGINE_DIR}/ResourceManager.cpp
    ${ENGINE_DIR}/ResourceManager.hpp
    ${ENGINE_DIR}/Scene.cpp
    ${ENGINE_DIR}/Scene.hpp
    ${ENGINE_DIR}/SceneParser.cpp
    ${ENGINE_DIR}/SceneParser.hpp
    ${ENGINE_DIR}/SceneState.hpp
    ${ENGINE_DIR}/ScriptEngine.hpp
    ${ENGINE_DIR}/ScriptTypeRegistry.hpp
    ${ENGINE_DIR}/Shader.cpp
    ${ENGINE_DIR}/Shader.hpp
    ${ENGINE_DIR}/ShaderManager.cpp
    ${ENGINE_DIR}/ShaderManager.hpp
    ${ENGINE_DIR}/ShadowPass.cpp
    ${ENGINE_DIR}/ShadowPass.hpp
    ${ENGINE_DIR}/StaticResources.cpp
    ${ENGINE_DIR}/StaticResources.hpp
    ${ENGINE_DIR}/Texture.cpp
    ${ENGINE_DIR}/Texture.hpp
    ${ENGINE_DIR}/TextureLoader.cpp
    ${ENGINE_DIR}/TextureLoader.hpp
    ${ENGINE_DIR}/TonemapEffect.cpp
    ${ENGINE_DIR}/TonemapEffect.hpp
    ${ENGINE_DIR}/TransformComponent.cpp
    ${ENGINE_DIR}/TransformComponent.hpp
    ${ENGINE_DIR}/TransformMatrices.hpp
    ${ENGINE_DIR}/Viewport.cpp
    ${ENGINE_DIR}/Viewport.hpp
    ${ENGINE_DIR}/Volatile.hpp
    ${ENGINE_DIR}/WaterMaterial.cpp
    ${ENGINE_DIR}/WaterMaterial.hpp
    ${ENGINE_DIR}/Window.cpp
    ${ENGINE_DIR}/Window.hpp
)
add_library(X::Engine ALIAS x)

message("${CMAKE_SOURCE_DIR}")

set(DX_LIBS
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
)

target_include_directories(x PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CODE_DIR}>
    $<BUILD_INTERFACE:${CODE_DIR}/Tools>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Engine/Shaders/Header>
    $<INSTALL_INTERFACE:include/XENGINE>
)

target_link_libraries(x
    PUBLIC
    ${DX_LIBS}
    assimp
    luajit
    sol2
    yaml-cpp
    ${BROTLI_LIBS}
    DirectXTex
    X::Pak
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DX_DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DX_RELEASE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Dist")
    add_definitions(-DX_DIST)
endif ()

target_compile_definitions(x PRIVATE ${WINDOWS_COMPILE_DEFS} LUA_JIT)