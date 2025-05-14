// Author: Jake Rieger
// Created: 4/14/2025.
//

// Things this needs to do:
// 1. Read in image data
// 2. Compress image data bytes with Brotli
// 3. Write the compressed bytes to a header file with the original size and compressed size

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <CLI/CLI.hpp>

#include "Common/Typedefs.hpp"
#include "Common/Filesystem.hpp"
#include "Tools/XPak/Compression.hpp"  // For Brotli helpers

int main(int argc, char* argv[]) {
    using namespace x;

    CLI::App app {"ResPak"};

    str outputFile;
    app.add_option("-o,--output", outputFile, "Output header file")->required();

    vector<str> inputFiles;
    app.add_option("-i,--input", inputFiles, "Input files")
      ->required()
      ->check(CLI::ExistingFile)  // Ensure files exist
      ->expected(1, -1);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) { return app.exit(e); }

    std::ofstream output(outputFile, std::ios::out);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file " << outputFile << std::endl;
        return EXIT_FAILURE;
    }

    output << "#pragma once\n\n";
    output << "// File generated with ResPak\n\n";

    for (const auto& input : inputFiles) {
        const auto imageFile = Path(input);
        str identifier       = imageFile.Filename();
        identifier           = identifier.substr(0, identifier.find_last_of("."));  // Remove extension
        std::ranges::transform(identifier, identifier.begin(), ::toupper);          // Force uppercase
        const str bytesIdentifier          = identifier + "_" + "BYTES";
        const str originalSizeIdentifier   = identifier + "_" + "ORIGINAL_SIZE";
        const str compressedSizeIdentifier = identifier + "_" + "COMPRESSED_SIZE";

        // Read image data
        i32 width, height, channels;
        stbi_uc* data = stbi_load(imageFile.CStr(), &width, &height, &channels, 0);
        if (!data) {
            std::cerr << "Failed to load image data for '" << imageFile.Str() << "'" << std::endl;
            continue;
        }
        const size_t originalSize = width * height * channels;
        std::cout << imageFile.Str() << std::endl;
        std::cout << " - Original size: " << originalSize << std::endl;

        vector<u8> originalBytes(originalSize);
        std::copy_n(data, originalSize, originalBytes.begin());
        stbi_image_free(data);
        X_ASSERT(originalSize == originalBytes.size());

        // Compress image data
        vector<u8> compressedBytes = BrotliCompression::Compress(originalBytes, 11);
        X_ASSERT(!compressedBytes.empty());

        std::cout << " - Compressed size: " << compressedBytes.size() << std::endl;
        std::cout << '\n';

        // Write to header file
        constexpr size_t bytesPerLine = 12;
        std::stringstream bytesStream;
        bytesStream
          << "//=========================================================================================//\n";
        bytesStream << "// " + identifier + "\n";
        bytesStream << "// Width: " + std::to_string(width) + ", Height: " + std::to_string(height) + "\n";
        bytesStream
          << "//=========================================================================================//\n";
        bytesStream << "static const unsigned char " + bytesIdentifier + "[] = {\n    ";

        size_t bytesRemaining = compressedBytes.size();
        size_t offset         = 0;

        while (bytesRemaining > 0) {
            size_t bytesToRead = (bytesRemaining < bytesPerLine) ? bytesRemaining : bytesPerLine;

            for (size_t i = 0; i < bytesToRead; i++) {
                bytesStream << "0x" << std::setfill('0') << std::setw(2) << std::hex
                            << CAST<int>(compressedBytes[offset + i]);

                if (offset + i + 1 < compressedBytes.size()) {
                    bytesStream << ", ";
                    if ((offset + i + 1) % bytesPerLine == 0) { bytesStream << "\n    "; }
                }
            }

            offset += bytesToRead;
            bytesRemaining -= bytesToRead;
        }

        bytesStream << "\n};\n\n";
        bytesStream << "static constexpr size_t " + originalSizeIdentifier + " = " + std::to_string(originalSize) +
                         ";\n\n";
        bytesStream << "static constexpr size_t " + compressedSizeIdentifier + " = " +
                         std::to_string(compressedBytes.size()) + ";\n\n";
        bytesStream << "static constexpr int " + identifier + "_WIDTH = " + std::to_string(width) + ";\n\n";
        bytesStream << "static constexpr int " + identifier + "_HEIGHT = " + std::to_string(height) + ";\n\n";

        bytesStream
          << "//=========================================================================================//\n";
        bytesStream
          << "//=========================================================================================//\n\n\n";

        output << bytesStream.str();
    }
}