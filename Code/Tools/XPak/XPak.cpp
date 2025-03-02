// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "XPak.hpp"

#include <numeric>
#include <brotli/encode.h>

#include "Compression.hpp"

namespace x {
    using namespace Filesystem;

    static void ProcessAssetDirectory(const Filesystem::Path& directory,
                                      vector<XPakTableEntry>& tableEntries,
                                      vector<XPakAssetEntry>& assetEntries) {
        for (auto& file : directory.Entries()) {
            if (file.IsFile()) {
                if (file.Extension() == "xasset") {
                    AssetDescriptor asset;
                    const auto result = asset.FromFile(file.Str());
                    if (!result) {
                        printf("Failed to load asset '%s'\n", file.CStr());
                        continue;
                    }

                    printf("Processing asset\n  | ID: %llu\n  | Type: %s\n  | Filename: %s\n",
                           asset.mId,
                           asset.TypeToString().c_str(),
                           asset.mFilename.c_str());

                    if (asset.mType == kAssetType_Invalid) {
                        printf("Invalid asset type '%s'\n", file.CStr());
                        continue;
                    }

                    XPakTableEntry tableEntry;
                    tableEntry.mAssetType = asset.mType;
                    tableEntry.mAssetId   = asset.mId;

                    const auto filename  = file.Parent() / asset.mFilename;
                    const auto assetData = FileReader::ReadAllBytes(filename);
                    XPakAssetEntry assetEntry;

                    if (asset.mType == kAssetType_Texture || asset.mType == kAssetType_Audio) {
                        // Textures are already compressed (DDS) and audio should not be compressed (WAV)
                        assetEntry.mCompressedData = assetData;
                        // Both textures and audio assets can be streamed, no decompression is required
                        tableEntry.mAssetFlags |= kAssetFlag_Streamable;
                    } else {
                        // Everything else gets compressed with Brotli (for now)
                        assetEntry.mCompressedData = BrotliCompression::Compress(assetData);
                        auto flags                 = kAssetFlag_Compressed;

                        if (asset.mType == kAssetType_Material || asset.mType == kAssetType_Scene) {
                            flags |= kAssetFlag_Descriptor;
                        }

                        tableEntry.mAssetFlags = flags;
                    }

                    // Update size members in ToC entry
                    tableEntry.mSize           = assetData.size();
                    tableEntry.mCompressedSize = assetEntry.mCompressedData.size();

                    tableEntries.push_back(tableEntry);
                    assetEntries.push_back(assetEntry);
                }
            }
        }
    }

    bool XPak::FromBytes(std::span<const u8> data) {
        return true;
    }

    std::vector<u8> XPak::ToBytes() const {
        // Calculate pak size
        constexpr auto headerSize = sizeof(XPakHeader);
        const auto tableSize      = mTableOfContents.size() * sizeof(XPakTableEntry);
        const size_t dataSize =
          std::accumulate(mAssets.begin(), mAssets.end(), 0, [](size_t size, const XPakAssetEntry& entry) {
              const size_t entrySize = sizeof(entry.mMagic) + entry.mPadding.size() + entry.mCompressedData.size();
              return size + entrySize;
          });
        const auto pakSize = headerSize + tableSize + dataSize;
        size_t offset      = 0;
        std::vector<u8> pak(pakSize);

        std::copy_n(mHeader.ToBytes().data(), headerSize, pak.data());
        offset += headerSize;

        vector<u8> tableBytes(tableSize);
        for (size_t i = 0; i < mTableOfContents.size(); i++) {
            size_t entryOffset = kTableEntrySize * i;
            auto& tableEntry   = mTableOfContents[i];
            std::copy_n(tableEntry.ToBytes().data(), kTableEntrySize, tableBytes.data() + entryOffset);
        }

        std::copy_n(tableBytes.data(), tableSize, pak.data() + offset);
        offset += tableSize;

        vector<u8> dataBytes(dataSize);
        size_t dataOffset = 0;
        for (size_t i = 0; i < mAssets.size(); i++) {
            auto& assetEntry      = mAssets[i];
            const auto assetBytes = assetEntry.ToBytes();
            std::copy_n(assetBytes.data(), assetBytes.size(), dataBytes.data() + dataOffset);
            dataOffset += assetBytes.size();
        }

        std::copy_n(dataBytes.data(), dataSize, pak.data() + offset);

        printf(" - Created pak file.\n");
        printf(" - Total size: %zu bytes (%zu MB)\n", pakSize, dataSize / (1024 * 1024));

        return pak;
    }

    std::optional<XPak> XPak::Create(const ProjectDescriptor& project) {
        XPak x;
        auto& header    = x.mHeader;
        header.mVersion = kCurrentVersion;

        // Parse project directories to scan for assets
        auto contentDir   = Path(project.mContentDirectory);
        auto scriptsDir   = Path(project.mScriptsDirectory);
        auto materialsDir = Path(project.mMaterialsDirectory);
        auto scenesDir    = Path(project.mScenesDirectory);

        ProcessAssetDirectory(contentDir, x.mTableOfContents, x.mAssets);
        ProcessAssetDirectory(scriptsDir, x.mTableOfContents, x.mAssets);
        ProcessAssetDirectory(materialsDir, x.mTableOfContents, x.mAssets);
        ProcessAssetDirectory(scenesDir, x.mTableOfContents, x.mAssets);

        if (x.mAssets.size() != x.mTableOfContents.size()) {
            printf("Incorrect number of assets in table of contents\n");
            return std::nullopt;
        }

        header.mEntries    = x.mAssets.size();
        size_t assetOffset = sizeof(XPakHeader) + (header.mEntries * sizeof(XPakTableEntry));

        for (size_t i = 0; i < header.mEntries; ++i) {
            // Update asset offset
            x.mTableOfContents[i].mOffset = assetOffset;

            const auto currentSize = x.mAssets[i].mCompressedData.size() + kAssetHeaderSize;
            if (currentSize % kAssetByteAlignment != 0) {
                // Calculate appropriate padding
                const auto paddingNeeded = kAssetByteAlignment - (currentSize % kAssetByteAlignment);
                x.mAssets[i].mPadding    = vector<u8>(paddingNeeded, 0);

                assetOffset += currentSize + paddingNeeded;
            } else {
                assetOffset += currentSize;
            }
        }

        printf("\n");
        printf(" - Processed %llu assets.\n", x.mAssets.size());

        return x;
    }
}  // namespace x