// Author: Jake Rieger
// Created: 3/2/2025.
//
// Header:
//   'XPAK': 4 bytes
//   Version: 2 bytes
//   Flags: 2 bytes
//   Entry count: 64-bit unsigned
//   Reserved: 16 bytes
//
// Table of Contents (per entry):
//   Asset ID: 64-bit unsigned, type embedded in highest 8 bits, ID is 56 bits
//   Asset flags: 16-bit (compressed, encrypted, streamable)
//   Asset data offset: 64-bit unsigned
//   Asset data size (compressed): 64-bit unsigned
//   Asset data size (original): 64-bit unsigned
//   Asset name hash: 32-bit unsigned (for string-based lookups)
//   Checksum: 32-bit CRC32 (or 64-bit for better collision avoidance)
//
// Assets (aligned to 64-byte boundaries):
//   [Asset Data Block]
//     Asset Header (16 bytes):
//       Magic: 4 bytes (e.g., 'ASET')
//       Flags: 4 bytes
//       Metadata size: 8 bytes
//     [Optional Metadata Block]
//       JSON or binary metadata
//     [Data Block]
//       Compressed/raw asset data
//     [Padding]
//       Padding to 64-byte alignment
//
//
//  TODO: Embed asset type in high bits of asset ID
//  For example, use 56 bits for the ID and 8 bits for the type information

#pragma once

#include <format>
#include <span>

#include "AssetDescriptor.hpp"
#include "ProjectDescriptor.hpp"
#include "Common/Types.hpp"

#define X_ARRAY_PADDING(sizeInBytes) unsigned char mPadding[sizeInBytes] {0};

namespace x {
    static constexpr u16 kCurrentVersion        = 1;
    static constexpr size_t kAssetHeaderSize    = 4;
    static constexpr size_t kAssetByteAlignment = 64;
    static constexpr size_t kTableEntrySize     = 64;

    static constexpr u16 kAssetFlag_Compressed = 1 << 0;
    static constexpr u16 kAssetFlag_Encrypted  = 1 << 1;
    static constexpr u16 kAssetFlag_Streamable = 1 << 2;
    static constexpr u16 kAssetFlag_Descriptor = 1 << 3;

    struct XPakHeader {
        const char mMagic[4] {'X', 'P', 'A', 'K'};
        u16 mVersion {0};
        u16 mFlags {0};
        u64 mEntries {0};
        X_ARRAY_PADDING(16)

        bool FromBytes(std::span<const u8> data);
        std::vector<u8> ToBytes() const;
        std::string ToString() const;
    };

    struct XPakTableEntry {
        AssetId mAssetId {0};
        u16 mAssetFlags {0};
        u64 mOffset {0};
        u64 mCompressedSize {0};
        u64 mSize {0};
        X_ARRAY_PADDING(24)  // u16 has 6 bits of padding, aligned to 8 bits = 40 bits total

        bool FromBytes(std::span<const u8> data);
        std::vector<u8> ToBytes() const;
        std::string ToString() const;
    };

    struct XPakAssetEntry {
        const char mMagic[4] {'A', 'S', 'E', 'T'};
        std::vector<u8> mCompressedData;
        std::vector<u8> mPadding;  // Each asset entry is padded to 64-byte boundaries, calculated when packing

        bool FromBytes(std::span<const u8> data);
        std::vector<u8> ToBytes() const;
        std::string ToString() const;
    };

    using AssetTable = std::unordered_map<AssetId, XPakTableEntry>;

    class XPak {
    public:
        XPak() = default;

        bool FromBytes(std::span<const u8> data);
        std::vector<u8> ToBytes() const;

        static AssetTable ReadPakTable(const Filesystem::Path& pakFile);
        static AssetTable ReadPakTable(std::span<const u8> data);
        static vector<u8> FetchAssetData(const Filesystem::Path& pakFile, const XPakTableEntry& entry);
        static std::optional<XPak> Create(const ProjectDescriptor& project);

    private:
        XPakHeader mHeader;
        std::vector<XPakTableEntry> mTableOfContents;
        std::vector<XPakAssetEntry> mAssets;
    };
}  // namespace x

#undef X_ARRAY_PADDING