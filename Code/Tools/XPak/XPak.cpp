// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "XPak.hpp"
#include "Compression.hpp"
#include "ScriptCompiler.hpp"

#include <iostream>
#include <numeric>
#include <brotli/encode.h>

namespace x {
    static void ProcessAssetDirectory(const Path& directory,
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
                           asset.GetTypeString().c_str(),
                           asset.mFilename.c_str());

                    auto assetType = asset.GetTypeFromId();

                    if (assetType == kAssetType_Invalid) {
                        printf("Invalid asset type '%s'\n", file.CStr());
                        continue;
                    }

                    XPakTableEntry tableEntry;
                    tableEntry.mAssetId = asset.mId;

                    const auto filename  = file.Parent() / asset.mFilename;
                    const auto assetData = FileReader::ReadAllBytes(filename);
                    XPakAssetEntry assetEntry;

                    if (assetType == kAssetType_Texture || assetType == kAssetType_Audio) {
                        // Textures are already compressed (DDS) and audio should not be compressed (WAV)
                        assetEntry.mCompressedData = assetData;
                        // Both textures and audio assets can be streamed, no decompression is required
                        tableEntry.mAssetFlags |= kAssetFlag_Streamable;

                        tableEntry.mSize           = assetData.size();
                        tableEntry.mCompressedSize = tableEntry.mSize;
                    } else if (assetType == kAssetType_Script) {
                        // Scripts can get compiled to bytecode
                        const str scriptSource    = FileReader::ReadAllText(filename);
                        vector<u8> scriptBytecode = ScriptCompiler::Compile(scriptSource, filename.Str());
                        if (scriptBytecode.size() == 0) {
                            printf("Failed to compile lua bytecode for script '%s'\n", filename.CStr());
                        }
                        assetEntry.mCompressedData = scriptBytecode;
                        tableEntry.mAssetFlags     = 0;

                        tableEntry.mSize           = scriptBytecode.size();
                        tableEntry.mCompressedSize = tableEntry.mSize;
                    } else {
                        // Everything else gets compressed with Brotli (for now)
                        assetEntry.mCompressedData = BrotliCompression::Compress(assetData);
                        auto flags                 = kAssetFlag_Compressed;

                        if (assetType == kAssetType_Material || assetType == kAssetType_Scene) {
                            flags |= kAssetFlag_Descriptor;
                        }

                        tableEntry.mAssetFlags = flags;

                        tableEntry.mSize           = assetData.size();
                        tableEntry.mCompressedSize = assetEntry.mCompressedData.size();
                    }

                    tableEntries.push_back(tableEntry);
                    assetEntries.push_back(assetEntry);
                }
            }
        }
    }

    bool XPakHeader::FromBytes(std::span<const u8> data) {
        if (data.size() != sizeof(XPakHeader)) {
            std::cerr << "XPakHeader::FromBytes: Invalid size " << data.size() << std::endl;
            return false;
        }

        size_t offset = 0;

        const auto magicSpan = data.subspan(offset, sizeof(mMagic));
        const auto magic     = RCAST<const char*>(magicSpan.data());
        for (int i = 0; i < 4; ++i) {
            if (magic[i] != mMagic[i]) {
                std::cerr << "XPakHeader::FromBytes: Invalid magic value " << magic[i] << std::endl;
                return false;
            }
        }
        offset += sizeof(mMagic);

        const auto versionSpan = data.subspan(offset, sizeof(mVersion));
        const auto version     = *(RCAST<const u16*>(versionSpan.data()));
        if (version != kCurrentVersion) {
            std::cerr << "XPakHeader::FromBytes: Invalid version " << version << std::endl;
            return false;
        }
        mVersion = version;
        offset += sizeof(mVersion);

        const auto flagsSpan = data.subspan(offset, sizeof(mFlags));
        const auto flags     = *(RCAST<const u16*>(flagsSpan.data()));
        offset += sizeof(mFlags);
        // Do nothing for now as no flags have been defined

        const auto entriesSpan = data.subspan(offset, sizeof(mEntries));
        const auto entries     = *(RCAST<const u64*>(entriesSpan.data()));
        mEntries               = entries;

        return true;
    }

    std::vector<u8> XPakHeader::ToBytes() const {
        std::vector<u8> data(sizeof(XPakHeader), 0);
        size_t offset = 0;

        std::copy_n(RCAST<const u8*>(mMagic), sizeof(mMagic), data.data() + offset);
        offset += sizeof(mMagic);

        std::copy_n(RCAST<const u8*>(&mVersion), sizeof(mVersion), data.data() + offset);
        offset += sizeof(mVersion);

        std::copy_n(RCAST<const u8*>(&mFlags), sizeof(mFlags), data.data() + offset);
        offset += sizeof(mFlags);

        std::copy_n(RCAST<const u8*>(&mEntries), sizeof(mEntries), data.data() + offset);
        // offset += sizeof(mEntries);

        return data;
    }

    std::string XPakHeader::ToString() const {
        char magicBuffer[5] = {'\0'};
        std::copy_n(mMagic, sizeof(mMagic), magicBuffer);  // null terminators are awesome! 🙄
        return std::format("Magic: {}, Version: {}, Flags: {}, Entries: {}", magicBuffer, mVersion, mFlags, mEntries);
    }

    bool XPakTableEntry::FromBytes(std::span<const u8> data) {
        if (data.size() != sizeof(XPakTableEntry)) {
            std::cerr << "XPakTableEntry::FromBytes: Invalid size " << data.size() << std::endl;
            return false;
        }

        size_t offset = 0;
        auto idSpan   = data.subspan(offset, sizeof(mAssetId));
        offset += sizeof(mAssetId);
        auto flagsSpan = data.subspan(offset, sizeof(mAssetFlags));
        offset += sizeof(mAssetFlags);
        auto offsetSpan = data.subspan(offset, sizeof(mOffset));
        offset += sizeof(mOffset);
        auto compressedSizeSpan = data.subspan(offset, sizeof(mCompressedSize));
        offset += sizeof(mCompressedSize);
        auto sizeSpan = data.subspan(offset, sizeof(mSize));

        mAssetId        = *RCAST<const u64*>(idSpan.data());
        mAssetFlags     = *RCAST<const u16*>(flagsSpan.data());
        mOffset         = *RCAST<const u64*>(offsetSpan.data());
        mCompressedSize = *RCAST<const u64*>(compressedSizeSpan.data());
        mSize           = *RCAST<const u64*>(sizeSpan.data());

        return true;
    }

    std::vector<u8> XPakTableEntry::ToBytes() const {
        std::vector<u8> data(sizeof(XPakTableEntry));
        size_t offset = 0;

        std::copy_n(RCAST<const u8*>(&mAssetId), sizeof(mAssetId), data.data() + offset);
        offset += sizeof(mAssetId);

        std::copy_n(RCAST<const u8*>(&mAssetFlags), sizeof(mAssetFlags), data.data() + offset);
        offset += sizeof(mAssetFlags);

        std::copy_n(RCAST<const u8*>(&mOffset), sizeof(mOffset), data.data() + offset);
        offset += sizeof(mOffset);

        std::copy_n(RCAST<const u8*>(&mCompressedSize), sizeof(mCompressedSize), data.data() + offset);
        offset += sizeof(mCompressedSize);

        std::copy_n(RCAST<const u8*>(&mSize), sizeof(mSize), data.data() + offset);
        offset += sizeof(mSize);

        std::copy_n(RCAST<const u8*>(mPadding), sizeof(mPadding), data.data() + offset);

        return data;
    }

    std::string XPakTableEntry::ToString() const {
        const AssetType type = AssetDescriptor::GetTypeFromId(mAssetId);
        const str fmt        = std::format(
          "Asset:\n  ID: {}\n  Type: {}\n  Size: {} bytes\n  Compressed Size: {} bytes\n  Offset: {:#010x}\n",
          mAssetId,
          AssetDescriptor::GetTypeString(type),
          mCompressedSize,
          mSize,
          mOffset);
        return fmt;
    }

    bool XPakAssetEntry::FromBytes(std::span<const u8> data) {
        return true;
    }

    std::vector<u8> XPakAssetEntry::ToBytes() const {
        const size_t size = 4 + mCompressedData.size() + mPadding.size();
        std::vector<u8> data(size);
        size_t offset = 0;

        std::copy_n(RCAST<const u8*>(mMagic), sizeof(mMagic), data.data() + offset);
        offset += sizeof(mMagic);

        std::copy_n(RCAST<const u8*>(mCompressedData.data()), mCompressedData.size(), data.data() + offset);
        offset += mCompressedData.size();

        std::copy_n(RCAST<const u8*>(mPadding.data()), mPadding.size(), data.data() + offset);

        return data;
    }

    std::string XPakAssetEntry::ToString() const {
        return "";
    }

    bool XPak::FromBytes(std::span<const u8> data) {
        size_t offset   = 0;
        auto headerSpan = data.subspan(0, sizeof(XPakHeader));
        offset += sizeof(XPakHeader);

        mHeader.FromBytes(headerSpan);
        const u64 numEntries = mHeader.mEntries;

        size_t tableSize = sizeof(XPakTableEntry) * numEntries;
        auto tableSpan   = data.subspan(offset, tableSize);

        for (size_t i = 0; i < numEntries; i++) {
            XPakTableEntry tableEntry;
            tableEntry.FromBytes(tableSpan.subspan(i * sizeof(XPakTableEntry), sizeof(XPakTableEntry)));
            mTableOfContents.push_back(tableEntry);
        }

        offset += tableSize;

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

    AssetTable XPak::ReadPakTable(const Path& pakFile) {
        auto pakBytes = FileReader::ReadAllBytes(pakFile);
        return ReadPakTable(pakBytes);
    }

    AssetTable XPak::ReadPakTable(std::span<const u8> data) {
        AssetTable assetTable;

        size_t offset   = 0;
        auto headerSpan = data.subspan(0, sizeof(XPakHeader));
        offset += sizeof(XPakHeader);

        XPakHeader header;
        header.FromBytes(headerSpan);
        const u64 numEntries = header.mEntries;

        size_t tableSize = sizeof(XPakTableEntry) * numEntries;
        auto tableSpan   = data.subspan(offset, tableSize);

        for (size_t i = 0; i < numEntries; i++) {
            XPakTableEntry tableEntry;
            tableEntry.FromBytes(tableSpan.subspan(i * sizeof(XPakTableEntry), sizeof(XPakTableEntry)));
            assetTable[tableEntry.mAssetId] = tableEntry;
        }

        return assetTable;
    }

    vector<u8> XPak::FetchAssetData(const Path& pakFile, const XPakTableEntry& entry) {
        if (!pakFile.Exists()) {
            std::cerr << "File not found: " << pakFile.Str() << std::endl;
            return {};
        }

        bool compressed = CHECK_FLAG(entry.mAssetFlags, kAssetFlag_Compressed);
        auto bytes      = FileReader::ReadBlock(pakFile, entry.mCompressedSize, entry.mOffset + kAssetHeaderSize);
        if (bytes.size() != entry.mCompressedSize) {
            std::cerr << "File size mismatch: " << pakFile.Str() << std::endl;
            return {};
        }

        vector<u8> outBytes(entry.mSize);
        if (compressed) {
            auto decompressed = BrotliCompression::Decompress(bytes, entry.mSize);
            if (decompressed.size() != entry.mSize) {
                std::cerr << "File size mismatch: " << pakFile.Str() << std::endl;
                return {};
            }
            std::copy_n(decompressed.data(), decompressed.size(), outBytes.data());
        } else {
            // The compressed and uncompressed sizes should match in this instance
            if (entry.mSize != entry.mCompressedSize) {
                std::cerr << "File size mismatch: " << pakFile.Str() << std::endl;
                return {};
            }
            std::copy_n(bytes.data(), bytes.size(), outBytes.data());
        }

        return outBytes;
    }

    std::optional<XPak> XPak::Create(const ProjectDescriptor& project) {
        XPak x;
        auto& header    = x.mHeader;
        header.mVersion = kCurrentVersion;

        // Parse project directories to scan for assets
        // Directories are relative to the path of the project file
        const auto contentDir = Path(project.mContentDirectory);
        ProcessAssetDirectory(contentDir, x.mTableOfContents, x.mAssets);

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