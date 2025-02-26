#pragma once

namespace x::Memory {
    // Base sizes in bytes
    static constexpr size_t BYTES_64   = 64;
    static constexpr size_t BYTES_128  = 128;
    static constexpr size_t BYTES_256  = 256;
    static constexpr size_t BYTES_512  = 512;
    static constexpr size_t BYTES_1K   = 1024;
    static constexpr size_t BYTES_2K   = 2 * BYTES_1K;
    static constexpr size_t BYTES_4K   = 4 * BYTES_1K;  // Typical page size
    static constexpr size_t BYTES_8K   = 8 * BYTES_1K;
    static constexpr size_t BYTES_16K  = 16 * BYTES_1K;
    static constexpr size_t BYTES_32K  = 32 * BYTES_1K;
    static constexpr size_t BYTES_64K  = 64 * BYTES_1K;
    static constexpr size_t BYTES_128K = 128 * BYTES_1K;
    static constexpr size_t BYTES_256K = 256 * BYTES_1K;
    static constexpr size_t BYTES_512K = 512 * BYTES_1K;

    // Megabyte ranges
    static constexpr size_t BYTES_1MB   = 1024 * BYTES_1K;
    static constexpr size_t BYTES_2MB   = 2 * BYTES_1MB;
    static constexpr size_t BYTES_4MB   = 4 * BYTES_1MB;
    static constexpr size_t BYTES_8MB   = 8 * BYTES_1MB;
    static constexpr size_t BYTES_16MB  = 16 * BYTES_1MB;
    static constexpr size_t BYTES_32MB  = 32 * BYTES_1MB;
    static constexpr size_t BYTES_64MB  = 64 * BYTES_1MB;
    static constexpr size_t BYTES_128MB = 128 * BYTES_1MB;
    static constexpr size_t BYTES_256MB = 256 * BYTES_1MB;
    static constexpr size_t BYTES_512MB = 512 * BYTES_1MB;

    // Gigabyte ranges
    static constexpr size_t BYTES_1GB = 1024 * BYTES_1MB;
    static constexpr size_t BYTES_2GB = 2 * BYTES_1GB;
    static constexpr size_t BYTES_4GB = 4 * BYTES_1GB;
}  // namespace x::Memory