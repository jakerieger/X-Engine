#include "ArenaAllocator.hpp"

namespace x {
    size_t ArenaAllocator::AlignForward(size_t addr, size_t alignment) {
        return (addr + (alignment - 1)) & ~(alignment - 1);
    }

    ArenaAllocator::ArenaAllocator(size_t size): _totalSize(size) {
        _memory     = new u8[size];
        _currentPos = _memory;
    }

    ArenaAllocator::~ArenaAllocator() {
        delete[] _memory;
    }

    ArenaAllocator::ArenaAllocator(ArenaAllocator&& other) noexcept: _memory(other._memory),
                                                                     _currentPos(other._currentPos),
                                                                     _totalSize(other._totalSize) {
        other._memory     = None;
        other._currentPos = None;
        other._totalSize  = 0;
    }

    ArenaAllocator& ArenaAllocator::operator=(ArenaAllocator&& other) noexcept {
        if (this != &other) {
            delete[] _memory;

            _memory     = other._memory;
            _currentPos = other._currentPos;
            _totalSize  = other._totalSize;

            other._memory     = None;
            other._currentPos = None;
            other._totalSize  = 0;
        }

        return *this;
    }

    void* ArenaAllocator::Allocate(size_t size, size_t alignment) {
        size_t currentAddr = RCAST<size_t>(_currentPos);
        size_t alignAddr   = AlignForward(currentAddr, alignment);
        size_t adjustment  = alignAddr - currentAddr;

        if (_currentPos + size + adjustment > _memory + _totalSize) {
            return None; // out of memory :(
        }

        _currentPos = RCAST<u8*>(alignAddr + size);
        return RCAST<void*>(alignAddr);
    }

    void ArenaAllocator::Reset() {
        _currentPos = _memory;
    }

    size_t ArenaAllocator::GetUsedMemory() const {
        return CAST<size_t>(_currentPos - _memory);
    }

    size_t ArenaAllocator::GetSize() const {
        return _totalSize;
    }

    size_t ArenaAllocator::GetAvailableMemory() const {
        return _totalSize - GetUsedMemory();
    }
}