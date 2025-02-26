#include "ArenaAllocator.hpp"

namespace x {
    size_t ArenaAllocator::AlignForward(size_t addr, size_t alignment) {
        return (addr + (alignment - 1)) & ~(alignment - 1);
    }

    ArenaAllocator::ArenaAllocator(size_t size) : mTotalSize(size) {
        mMemory     = new u8[size];
        mCurrentPos = mMemory;
    }

    ArenaAllocator::~ArenaAllocator() {
        delete[] mMemory;
    }

    ArenaAllocator::ArenaAllocator(ArenaAllocator&& other) noexcept
        : mMemory(other.mMemory), mCurrentPos(other.mCurrentPos), mTotalSize(other.mTotalSize) {
        other.mMemory     = None;
        other.mCurrentPos = None;
        other.mTotalSize  = 0;
    }

    ArenaAllocator& ArenaAllocator::operator=(ArenaAllocator&& other) noexcept {
        if (this != &other) {
            delete[] mMemory;

            mMemory     = other.mMemory;
            mCurrentPos = other.mCurrentPos;
            mTotalSize  = other.mTotalSize;

            other.mMemory     = None;
            other.mCurrentPos = None;
            other.mTotalSize  = 0;
        }

        return *this;
    }

    void* ArenaAllocator::Allocate(size_t size, size_t alignment) {
        size_t currentAddr = RCAST<size_t>(mCurrentPos);
        size_t alignAddr   = AlignForward(currentAddr, alignment);
        size_t adjustment  = alignAddr - currentAddr;

        if (mCurrentPos + size + adjustment > mMemory + mTotalSize) {
            return None;  // out of memory :(
        }

        mCurrentPos = RCAST<u8*>(alignAddr + size);
        return RCAST<void*>(alignAddr);
    }

    void ArenaAllocator::Reset() {
        mCurrentPos = mMemory;
    }

    size_t ArenaAllocator::GetUsedMemory() const {
        return CAST<size_t>(mCurrentPos - mMemory);
    }

    size_t ArenaAllocator::GetSize() const {
        return mTotalSize;
    }

    size_t ArenaAllocator::GetAvailableMemory() const {
        return mTotalSize - GetUsedMemory();
    }
}  // namespace x