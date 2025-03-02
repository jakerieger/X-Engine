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
//   Asset ID: 64-bit unsigned
//   Asset type: 16-bit (image, sound, model, etc.)
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

#pragma once
#include <format>
#include <iostream>
#include <ostream>
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

        bool FromBytes(std::span<const u8> data) {
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

        std::vector<u8> ToBytes() const {
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

        std::string ToString() const {
            char magicBuffer[5] = {'\0'};
            std::copy_n(mMagic, sizeof(mMagic), magicBuffer);  // null terminators are awesome! 🙄
            return std::format("Magic: {}, Version: {}, Flags: {}, Entries: {}",
                               magicBuffer,
                               mVersion,
                               mFlags,
                               mEntries);
        }
    };

    struct XPakTableEntry {
        u64 mAssetId {0};
        u16 mAssetType {kAssetType_Invalid};
        u16 mAssetFlags {0};
        u64 mOffset {0};
        u64 mCompressedSize {0};
        u64 mSize {0};
        X_ARRAY_PADDING(24)

        bool FromBytes(std::span<const u8> data) {
            return true;
        }

        std::vector<u8> ToBytes() const {
            std::vector<u8> data(sizeof(XPakTableEntry));
            size_t offset = 0;

            std::copy_n(RCAST<const u8*>(&mAssetId), sizeof(mAssetId), data.data() + offset);
            offset += sizeof(mAssetId);

            std::copy_n(RCAST<const u8*>(&mAssetType), sizeof(mAssetType), data.data() + offset);
            offset += sizeof(mAssetType);

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

        std::string ToString() const {
            return "";
        }
    };

    struct XPakAssetEntry {
        const char mMagic[4] {'A', 'S', 'E', 'T'};
        std::vector<u8> mCompressedData;
        std::vector<u8> mPadding;  // Each asset entry is padded to 64-byte boundaries

        bool FromBytes(std::span<const u8> data) {
            return true;
        }

        std::vector<u8> ToBytes() const {
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

        std::string ToString() const {
            return "";
        }
    };

    class XPak {
    public:
        XPak() = default;

        bool FromBytes(std::span<const u8> data);
        std::vector<u8> ToBytes() const;

        static std::optional<XPak> Create(const ProjectDescriptor& project);

    private:
        XPakHeader mHeader;
        std::vector<XPakTableEntry> mTableOfContents;
        std::vector<XPakAssetEntry> mAssets;
    };
}  // namespace x

#undef X_ARRAY_PADDING