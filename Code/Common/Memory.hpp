#pragma once

#include "Types.hpp"

namespace x {
    enum class MemoryResult : u8 {
        Success,
        NullPtr,
        ZeroSize,
        BufferOverflow,
        OverlappingBuffers,
    };

    class Memory {
    public:
        static MemoryResult Copy(void* dst, const void* src, size_t size, size_t dstSize) {
            if (!dst || !src) { return MemoryResult::NullPtr; }
            if (size == 0 || dstSize == 0) { return MemoryResult::ZeroSize; }
            if (size > dstSize) { return MemoryResult::BufferOverflow; }

            const auto* srcPtr = CAST<const u8*>(src);
            const auto* dstPtr = CAST<const u8*>(dst);

            if ((srcPtr < dstPtr && srcPtr + size > dstPtr) || (dstPtr < srcPtr && dstPtr + size > srcPtr)) {
                return MemoryResult::OverlappingBuffers;
            }

            memcpy(dst, src, size);

            return MemoryResult::Success;
        }

        template<typename T>
        static MemoryResult CopyArray(T* dst, const T* src, const size_t count, const size_t dstCount) {
            return Copy(CAST<void*>(dst), CAST<const void*>(src), count * sizeof(T), dstCount * sizeof(T));
        }
    };
}  // namespace x