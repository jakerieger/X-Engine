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

#define X_PANIC_ASSERT(cond, fmt, ...) \
    if (!(cond)) x::impl::Panic(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#define X_NODISCARD [[nodiscard]]

// I know you can also use _CONSOLE to check if the subsystem is console or not, but I have other things that'll change
// depending on whether the current configuration is set to Distribution.
#if defined(X_DISTRIBUTION)
#define X_MAIN int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
#define X_MODULE_HANDLE hInstance
#else
#define X_MAIN int main(int argc, char* argv[])
#define X_MODULE_HANDLE GetModuleHandleA(nullptr)
#endif