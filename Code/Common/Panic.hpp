#pragma once

#include <comdef.h>

namespace x::impl {
    [[noreturn]] inline void Panic(const char* file, int line, const char* func, const char* msg) noexcept {
        char msgFormatted[2048];
        std::ignore = snprintf(msgFormatted, 2048, "%s:%d - Panic in `%s()` :\n - %s\n", file, line, func, msg);
        std::ignore = fprintf(stderr, CAST<const char*>(msgFormatted));
#ifndef NDEBUG
        OutputDebugStringA(msgFormatted);
#endif
        std::abort();
    }

    template<typename... Args>
    [[noreturn]] void Panic(const char* file, int line, const char* func, const char* fmt, Args... args) noexcept {
        char msg[1024];
        std::ignore = snprintf(msg, sizeof(msg), fmt, args...);
        Panic(file, line, func, msg);
    }
}  // namespace x::impl

#define PANIC(fmt, ...) x::impl::Panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define PANIC_IF_FAILED(hr, fmt, ...)                                                                                  \
    if (FAILED(hr)) { x::impl::Panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); }