// Author: Jake Rieger
// Created: 3/2/2025.
//

#include <assert.h>

#include "Common/Filesystem.hpp"
#include "ProjectDescriptor.hpp"
#include "AssetGenerator.hpp"
#include "XPak.hpp"
#include <ranges>
#include <CLI/CLI.hpp>

using namespace x;

struct PackArgs {
    str mProjectFile;
    str mPakName = "Data.xpak";
};

struct UnpackArgs {
    str mPakFile;
    str mOutputDir = "./";
};

struct GenAssetArgs {
    str mAssetFile;
    str mAssetType;
};

struct DumpArgs {
    str mPakFile;
};

int main(int argc, char* argv[]) {
    CLI::App app {"XPak CLI"};

    auto* pack = app.add_subcommand("pack", "Pack project assets into pak file");
    PackArgs packArgs;
    pack->add_option("project_file", packArgs.mProjectFile, "Project file path")->required(true);
    pack->add_option("-n,--name", packArgs.mPakName, "Output pak file name");

    auto* unpack = app.add_subcommand("unpack", "Unpack assets from pak file");
    UnpackArgs unpackArgs;
    unpack->add_option("pak_file", unpackArgs.mPakFile, "Pak file to unpack")->required(true);
    unpack->add_option("-o,--output", unpackArgs.mOutputDir, "Output directory");

    auto* generate = app.add_subcommand("generate", "Generates asset descriptor file for given source asset");
    GenAssetArgs genArgs;
    generate->add_option("asset_file", genArgs.mAssetFile, "Asset source file")->required(true);
    generate->add_option("-t,--type", genArgs.mAssetType, "Asset type (texture, mesh, material, etc.)");

    auto* dumpTable = app.add_subcommand("dump", "Dump pak file table contents");
    DumpArgs dumpArgs;
    dumpTable->add_option("pak_file", dumpArgs.mPakFile, "Pak file to dump")->required(true);

    app.require_subcommand(1);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) { return app.exit(e); }

    if (pack->parsed()) {
        Path project(packArgs.mProjectFile);
        if (!project.Exists()) {
            std::cerr << "Could not open project file " << packArgs.mProjectFile << std::endl;
            return EXIT_FAILURE;
        }

        ProjectDescriptor projectDescriptor;
        projectDescriptor.FromFile(project);

        auto createResult = XPak::Create(projectDescriptor);
        if (!createResult.has_value()) {
            std::cerr << "Could not create pak file from project" << std::endl;
            return EXIT_FAILURE;
        }
        const auto& pak = createResult.value();
        auto pakFile    = pak.ToBytes();
        if (pakFile.size() == 0) {
            std::cerr << "Could not create pak file from project (size 0)" << std::endl;
            return EXIT_FAILURE;
        }

        FileWriter::WriteAllBytes(Path::Current() / packArgs.mPakName, pakFile);
    }

    else if (unpack->parsed()) {
        auto pakFile = Path(unpackArgs.mPakFile);
        if (!pakFile.Exists()) {
            std::cerr << "Could not open pak file " << unpackArgs.mPakFile << std::endl;
            return EXIT_FAILURE;
        }

        auto assetTable = XPak::ReadPakTable(pakFile);
        if (assetTable.size() == 0) {
            std::cerr << "Could not read pak file" << std::endl;
            return EXIT_FAILURE;
        }

        auto outputDir = Path::Current() / unpackArgs.mOutputDir;
        if (!outputDir.Exists()) {
            if (!outputDir.CreateAll()) {
                std::cerr << "Could not create output directory" << std::endl;
                return EXIT_FAILURE;
            }
        }

        for (const auto& [id, asset] : assetTable) {
            auto assetName  = std::to_string(id) + ".bin";
            auto outputFile = Path(outputDir / assetName);
            auto assetData  = XPak::FetchAssetData(pakFile, asset);
            if (assetData.size() == 0) {
                std::cerr << "Could not fetch asset data from pak file" << std::endl;
                return EXIT_FAILURE;
            }
            FileWriter::WriteAllBytes(outputFile, assetData);
        }
    }

    else if (generate->parsed()) {
        auto sourceFile = Path(genArgs.mAssetFile);
        if (!sourceFile.Exists()) {
            std::cerr << "Could not open source file" << std::endl;
            return EXIT_FAILURE;
        }

        AssetType assetType = kAssetType_Invalid;
        if (genArgs.mAssetType.empty()) {
            // Attempt to detect the type based on the extension
            if (sourceFile.Extension() == "dds") assetType = kAssetType_Texture;
            else if (sourceFile.Extension() == "glb") assetType = kAssetType_Mesh;
            else if (sourceFile.Extension() == "lua") assetType = kAssetType_Script;
            else if (sourceFile.Extension() == "material") assetType = kAssetType_Material;
            else if (sourceFile.Extension() == "scene") assetType = kAssetType_Scene;
            else if (sourceFile.Extension() == "wav") assetType = kAssetType_Audio;

        } else if (genArgs.mAssetType == "texture") {
            assetType = kAssetType_Texture;
        } else if (genArgs.mAssetType == "mesh") {
            assetType = kAssetType_Mesh;
        } else if (genArgs.mAssetType == "audio") {
            assetType = kAssetType_Audio;
        } else if (genArgs.mAssetType == "material") {
            assetType = kAssetType_Material;
        } else if (genArgs.mAssetType == "scene") {
            assetType = kAssetType_Scene;
        } else if (genArgs.mAssetType == "script") {
            assetType = kAssetType_Script;
        }

        if (assetType == kAssetType_Invalid) {
            // Just ignore this file
            return 0;
        }

        if (!AssetGenerator::GenerateAsset(sourceFile, assetType, Path::Current())) {
            std::cerr << "Could not generate asset" << std::endl;
            return EXIT_FAILURE;
        }
    }

    else if (dumpTable->parsed()) {
        auto pakFile = Path(dumpArgs.mPakFile);
        if (!pakFile.Exists()) {
            std::cerr << "Could not open pak file" << std::endl;
            return EXIT_FAILURE;
        }

        auto assetTable = XPak::ReadPakTable(pakFile);
        if (assetTable.size() == 0) {
            std::cerr << "Could not read pak file" << std::endl;
            return EXIT_FAILURE;
        }

        for (const auto& asset : assetTable | std::views::values) {
            std::cout << asset.ToString() << std::endl;
        }
    }
}