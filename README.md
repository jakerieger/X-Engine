# Space Game

A protoype space exploration game/engine built using DirectX 11.

## Project Structure

All of the C++ code is in the aptly named directory [Code](Code). Shader sources can be found in [Engine/Shaders/Source](Engine/Shaders/Source).

The actual game executable code is in [Code/Game/main.cpp](Code/Game/main.cpp).

## Building

Building requires Visual Studio 2022. This is a Windows-only project since it uses DirectX 11.
There are no plans currently to port this to other platforms.

### Dependencies

Most of the third party dependencies are included in the directory `Code/Vendor`.
However, [Engine](Engine) requires DirectXTex which must be installed through NuGet.

All you need to do is set [Game](Game) as the startup project and build.