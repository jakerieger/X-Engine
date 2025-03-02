// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include "Common/Types.hpp"
#include <span>
#include <brotli/encode.h>

namespace x {
    class BrotliCompression {
    public:
        static std::vector<u8> Compress(std::span<const u8> data,
                                        int quality    = BROTLI_DEFAULT_QUALITY,
                                        int windowSize = BROTLI_DEFAULT_WINDOW);
        static std::vector<u8> Decompress(std::span<const u8> data, size_t expectedSize = 0);
    };
}  // namespace x
