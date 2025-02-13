# Space Game

A protoype space exploration game/engine built using DirectX 11.

## Project Structure

All of the C++ code is in the aptly named directory [Code](Code). Shader sources can be found in [Engine/Shaders/Source](Engine/Shaders/Source).

The actual game executable code is in [Code/Game/main.cpp](Code/Game/main.cpp).

## Engine Architecture

If you'd like to know how the actual "engine" part of this code-base works, you can check out the [Engine Architecture](Docs/Engine-Architecture.md) doc.

## Building

### Building requires [Visual Studio 2022](https://visualstudio.microsoft.com/vs/). This is a **Windows-only** project since it uses DirectX 11 and the Win32 API 😔.
### ⚠️ **There are no plans currently to port this to other platforms.** ⚠️

---

### Quickstart

1. Clone repository with submodules
2. Run [CONFIGURE_VS2022.bat](CONFIGURE_VS2022.bat) from project root (or just double-click it)
3. Open [SpaceGame.sln](SpaceGame.sln) and set `Game` as the startup project
4. Right-click the `Engine` project, go to **Manage NuGet Packages...** and install [directxtex_desktop_win10](https://www.nuget.org/packages/directxtex_desktop_win10)
    - *I'm not sure if it's possible to automate this but if so, I'll add it to the configuration scripts.*
5. Select a configuration (*Debug*, *Release*, or *Distribution*), **and build**.

---


**Example:**
```sh
git clone https://github.com/jakerieger/SpaceGame.git --recurse-submodules -j8
```
> `-j8` lets us use up to 8 threads to download repositories in parallel.

### Dependencies

Dependencies that are code-only can be found in [Code/Vendor](Code/Vendor). Dependencies that require building and linking are included as git submodules and can be found in [Vendor](Vendor).
The fastest way to get up and running is by simply executing [CONFIGURE_VS2022.bat](CONFIGURE_VS2022.bat) from the project root directory. This will automatically build and configure any of the vendor libraries.

If you prefer to do things manually, follow these steps:

1. Build [assimp](https://github.com/assimp/assimp) as a **static library** for x64
2. Build [LuaJIT](http://luajit.org/) with the **msvcbuild.bat** script found in `Vendor/luajit/src`, again for x64.
3. See **step 3** of the [Quickstart](#quickstart) for installing [DirectXTex](https://github.com/microsoft/DirectXTex)

Once the necessary dependencies have been built, open [SpaceGame.sln](SpaceGame.sln) and set `Game` as the startup project. Then just set a configuration (Debug, Release, or Distribution) and build!

## Docs

- [Engine Architecture](Docs/Engine-Architecture.md)
    - Basic overview of how the engine is designed and structured. 
- [Developer Console](Docs/Developer-Console.md)
    - Documentation for in-game developer console. 
- [Design Doc](Docs/Design-Doc.md)
    - Game outline and design goals. 