# Space Game

A protoype space exploration game/engine built using DirectX 11.

## Project Structure

All of the C++ code is in the aptly named directory [Code](Code). Shader sources can be found in [Engine/Shaders/Source](Engine/Shaders/Source).

The actual game executable code is in [Code/Game/main.cpp](Code/Game/main.cpp).

## Engine Architecture

If you'd like to know how the actual "engine" part of this code-base works, you can check out the [Engine Architecture](Docs/Engine-Architecture.md) doc.

## Building
This project uses Git Submodules, so make sure to use `--recurse-submodules` when doing `git clone`.

**Example:**
```sh
$ git clone https://github.com/jakerieger/SpaceGame.git --recurse-submodules --j8
```
> `-j8` lets us use up to 8 threads to download repositories in parallel.
---
### Building requires [Visual Studio 2022](https://visualstudio.microsoft.com/vs/). This is a **Windows-only** project since it uses DirectX 11 and the Win32 API 😔.
### ⚠️ **There are no plans currently to port this to other platforms.** ⚠️
---
### Dependencies

Dependencies that are code-only can be found in [Code/Vendor](Code/Vendor). Dependencies that require building and linking are included as git submodules and can be found in [Vendor](Vendor).

## Screenshots

![](Docs/ShadowMaps.png)
> **Shadow Maps (hard)**
>
> *Yes, I know the frame time calculation is wrong. Will fix sometime before I die.*
