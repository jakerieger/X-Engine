#pragma once

#include "EngineCommon.hpp"
#include "Common/Typedefs.hpp"

namespace x {
    /// @brief Heap-allocates a fixed size block of memory and returns a chunk of that memory when an allocation is
    /// requested. All memory gets freed at once.
    class ArenaAllocator {
        u8* mMemory;
        u8* mCurrentPos;
        size_t mTotalSize;
        static constexpr size_t kMinAlignment = alignof(std::max_align_t);

        static size_t AlignForward(size_t addr, size_t alignment);

        X_CLASS_PREVENT_COPIES(ArenaAllocator)

    public:
        ArenaAllocator(size_t size);
        ~ArenaAllocator();
        ArenaAllocator(ArenaAllocator&& other) noexcept;
        ArenaAllocator& operator=(ArenaAllocator&& other) noexcept;

        /// @brief Allocates memory of specified size with specified alignment if provided
        void* Allocate(size_t size, size_t alignment = kMinAlignment);

        /// @brief Allocates memory of size T for specified number of T
        template<typename T>
        T* AllocateType(size_t count = 1) {
            return CAST<T*>(Allocate(sizeof(T) * count, alignof(T)));
        }

        void Reset();

        /// @brief Returns memory in use by the arena in bytes
        size_t GetUsedMemory() const;
        /// @brief Returns the total size of the arena in bytes
        size_t GetSize() const;
        /// @brief Returns the available (free) memory in the arena in bytes
        size_t GetAvailableMemory() const;
    };
}  // namespace x