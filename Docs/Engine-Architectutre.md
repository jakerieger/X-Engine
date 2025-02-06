# Engine Architecture

The engine doesn't have a name, so I'll simply be referring to it as `x` since that's the namespace the engine code exists within.

As it currently stands, x is a rather simple forward rendered 3D engine with an Entity Component System (ECS).

The root class of the engine is the [IGame](Code/Engine/Game.hpp) class. This class is responsible for the following:

1. Initializing and creating the window
2. Initializing the rendering backend ([RenderContext](Code/Engine/RenderContext.hpp), [RenderSystem](Code/Engine/RenderSystem.hpp)).
3. Managing the game state ([GameState](Code/Engine/GameState.hpp)):
	- *At the moment there is no "scene" system. All game state is globally owned and managed by the `IGame` instance. Although convenient for testing purposes, this is a terrible design in practice and will not remain like this.*
4. Running the core game loop
5. Shutting down and cleaning up

## Rendering

As mentioned above, x uses a forward renderer. This simply means all the lighting calculations are done **per fragment**, and although simple to implement can lead to performance deficits with scenes containing a large number of lights since each fragment needs to calculate lighting for every light despite the fact that it might not even be visible or within range of said light. The plan for the future is to move to a hybrid forward+/deferred system.

### Passes

The scene is rendered for each pass except for the final post-processing pass, which takes the output of the lit pass and renders that to a fullscreen quad.

1. Shadow pass (depth-only)
2. Fully-lit pass
3. Post-processing pass

This can be seen with the following lines of code found in [Game.cpp](Code/Engine/Game.cpp):

```cpp
_renderSystem->BeginFrame();
{
    // Do our depth-only shadow pass first
    ID3D11ShaderResourceView* depthSRV;
    {
        _renderSystem->BeginShadowPass();
        RenderDepthOnly();
        depthSRV = _renderSystem->EndShadowPass();
    }

    // Do our fully lit pass using our previous depth-only pass as input for our shadow mapping shader
    ID3D11ShaderResourceView* sceneSRV;
    {
        _renderSystem->BeginLightPass(depthSRV);
        RenderScene();
        sceneSRV = _renderSystem->EndLightPass();
    }

    // We can now pass our fully lit scene texture to the post processing pipeline to be processed and displayed on screen
    {
        _renderSystem->PostProcessPass(sceneSRV);
    }
}
_renderSystem->EndFrame();
```

## What **isn't** implemented:

This is a list of things that are planned but not currently implemented:

1. Opaque/transparent mesh sorting
2. Anti-aliasing of any kind
3. Resource management
	- Resources are simply loaded from disk using hard-coded paths at the moment. No system for loading and managing these exists right now.
4. Scenes
5. Input handling
6. ...and a whole hell of a lot more :D