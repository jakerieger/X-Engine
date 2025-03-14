// Author: Jake Rieger
// Created: 3/11/2025.
//

#include "SceneEditor.hpp"

#undef X_MAIN
#undef X_MODULE_HANDLE
#define X_MAIN int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
#define X_MODULE_HANDLE hInstance

X_MAIN {
    x::SceneEditor sceneEditor;
    return sceneEditor.Run();
}