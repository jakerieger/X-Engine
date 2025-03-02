// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "Compression.hpp"
#include <stdexcept>
#include <brotli/decode.h>

namespace x {
    std::vector<u8> BrotliCompression::Compress(std::span<const u8> data, int quality, int windowSize) {
        const size_t maxCompressedSize = BrotliEncoderMaxCompressedSize(data.size());
        if (maxCompressedSize == 0) { throw std::runtime_error("Input data is too large for Brotli compression."); }

        std::vector<u8> result(maxCompressedSize);
        size_t encodedSize = maxCompressedSize;

        BROTLI_BOOL compressed = BrotliEncoderCompress(quality,
                                                       windowSize,
                                                       BROTLI_DEFAULT_MODE,
                                                       data.size(),
                                                       data.data(),
                                                       &encodedSize,
                                                       result.data());
        if (compressed == BROTLI_FALSE) { throw std::runtime_error("Compression failed."); }
        result.resize(encodedSize);
        return result;
    }

    std::vector<u8> BrotliCompression::Decompress(std::span<const u8> data, size_t expectedSize) {
        BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        if (!state) { throw std::runtime_error("Failed to create Brotli decoder state."); }

        std::vector<u8> result;
        if (expectedSize > 0) {
            result.resize(expectedSize);
        } else {
            result.resize(data.size() * 4);
        }

        size_t availableIn  = data.size();
        const u8* nextIn    = data.data();
        size_t availableOut = result.size();
        u8* nextOut         = result.data();

        size_t totalOut = 0;

        for (;;) {
            const BrotliDecoderResult decodeResult =
              BrotliDecoderDecompressStream(state, &availableIn, &nextIn, &availableOut, &nextOut, &totalOut);
            if (decodeResult == BROTLI_DECODER_RESULT_SUCCESS) {
                break;
            } else if (decodeResult == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
                const size_t currentPos = nextOut - result.data();
                result.resize(result.size() * 2);
                nextOut      = result.data() + currentPos;
                availableOut = result.size() - currentPos;
            } else {
                BrotliDecoderDestroyInstance(state);
                throw std::runtime_error("Failed to decode Brotli stream.");
            }
        }

        result.resize(totalOut);
        BrotliDecoderDestroyInstance(state);

        return result;
    }
}  // namespace x