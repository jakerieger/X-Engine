#pragma once

/// @brief Deletes both the move/copy assignment operator and constructor
#define CLASS_PREVENT_MOVES_COPIES(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME&) = delete; \
    CLASS_NAME(CLASS_NAME&&) = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&) = delete;

/// @brief Deletes the copy constructor and assignment operator
#define CLASS_PREVENT_COPIES(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME&) = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete;

/// @brief Deletes the move constructor and assignment operator
#define CLASS_PREVENT_MOVES(CLASS_NAME) \
    CLASS_NAME(CLASS_NAME&&) = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&) = delete;

#define CAST_DYNAMIC_AS \
    template<typename T> \
    T* As() { return dynamic_cast<T*>(this); }