// Author: Jake Rieger
// Created: 3/11/2025.
//

#include "XEditor.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    using namespace x;
    XEditor editor;
    return editor.Run();
}