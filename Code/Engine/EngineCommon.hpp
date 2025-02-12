#pragma once

/// @brief Deletes both the move/copy assignment operator and constructor
#define X_CLASS_PREVENT_MOVES_COPIES(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME&) = delete; \
    CLASS_NAME(CLASS_NAME&&) = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&) = delete;

/// @brief Deletes the copy constructor and assignment operator
#define X_CLASS_PREVENT_COPIES(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME&) = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete;

/// @brief Deletes the move constructor and assignment operator
#define X_CLASS_PREVENT_MOVES(CLASS_NAME) \
    CLASS_NAME(CLASS_NAME&&) = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&) = delete;

#define X_CAST_DYNAMIC_AS \
    template<typename T> \
    T* As() { return dynamic_cast<T*>(this); }

#define X_NODISCARD [[nodiscard]]

#if defined(X_DISTRIBUTION)

#define X_MAIN int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
#define X_MODULE_HANDLE hInstance
#define X_PANIC(fmt, ...)
#define X_PANIC_ASSERT(cond, fmt, ...)

#else

#define X_MAIN int main(int argc, char* argv[])
#define X_MODULE_HANDLE GetModuleHandleA(nullptr)

#include <comdef.h>
#include <cstdlib>

[[noreturn]] inline void Panic(const char* file, int line, const char* func, const char* msg) noexcept {
    char msgFormatted[2048];
    snprintf(msgFormatted, 2048, "%s:%d - Panic in `%s()` :\n - %s\n", file, line, func, msg);
    fprintf(stderr, static_cast<const char*>(msgFormatted));
    #ifndef NDEBUG
    OutputDebugStringA(msgFormatted);
    #endif
    std::abort();
}

template<typename... Args>
[[noreturn]] void Panic(const char* file, int line, const char* func, const char* fmt, Args... args) noexcept {
    char msg[1024];
    snprintf(msg, sizeof(msg), fmt, args...);
    Panic(file, line, func, msg);
}

#define X_PANIC(fmt, ...) Panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);
#define X_PANIC_ASSERT(cond, fmt, ...)                                                                             \
        if (!(cond)) Panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);

#endif