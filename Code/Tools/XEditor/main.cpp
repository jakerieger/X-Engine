// Author: Jake Rieger
// Created: 3/11/2025.
//

#include "XEditor.hpp"
#include "StartupScreen.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    using namespace x;

    // if (!XEditor::HasSession()) {
    //     StartupScreen startupScreen;
    //     startupScreen.Run();
    // }

    XEditor editor;
    return editor.Run();
}