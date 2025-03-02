// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "Common/Filesystem.hpp"
#include "ProjectDescriptor.hpp"
#include "XPak.hpp"

using namespace x;
using namespace x::Filesystem;

int main(int argc, char* argv[]) {
    ProjectDescriptor project;
    if (!project.FromFile(R"(C:\Users\conta\Code\SpaceGame\Game\SpaceGame.xproj)")) { return EXIT_FAILURE; }

    auto xpakResult = XPak::Create(project);
    if (xpakResult.has_value()) {
        auto xpak = *xpakResult;
        FileWriter::WriteAllBytes(Path("testPak.bin"), xpak.ToBytes());
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}