#pragma once

#define CLASS_PREVENT_MOVES_COPIES(CLASS_NAME) \
    CLASS_NAME(const CLASS_NAME& other) = delete; \
    CLASS_NAME(CLASS_NAME&& other) = delete; \
    CLASS_NAME& operator=(const CLASS_NAME& other) = delete; \
    CLASS_NAME& operator=(CLASS_NAME&& other) = delete;

#define CAST_DYNAMIC_AS \
    template<typename T> \
    T* As() { return dynamic_cast<T*>(this); }