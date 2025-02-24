// Author: Jake Rieger
// Created: 2/24/2025.
//

#pragma once

#include <variant>
#include <utility>
#include <stdexcept>

template<typename T>
struct Ok_T {
    T value;
    explicit Ok_T(const T& v) : value(v) {}
    explicit Ok_T(T&& v) : value(std::move(v)) {}
};

template<typename E>
struct Err_T {
    E value;
    explicit Err_T(const E& v) : value(v) {}
    explicit Err_T(E&& v) : value(std::move(v)) {}
};

template<typename T>
Ok_T<T> Ok(T&& value) {
    return Ok_T<T>(std::forward<T>(value));
}

template<typename E>
Err_T<E> Err(E&& error) {
    return Err_T<E>(std::forward<E>(error));
}

template<typename T, typename E>
class Result {
    std::variant<T, E> result;

public:
    template<typename U>
    Result(Ok_T<U> ok) : result(std::move(ok.value)) {}

    template<typename F>
    Result(Err_T<F> err) : result(std::move(err.value)) {}

    bool IsOk() const {
        return std::holds_alternative<T>(result);
    }

    bool IsErr() const {
        return std::holds_alternative<E>(result);
    }

    T OkValue() const {
        return std::get<T>(result);
    }

    E ErrValue() const {
        return std::get<E>(result);
    }

    T Unwrap() const {
        if (IsErr()) { throw std::runtime_error("Called Unwrap on an error value"); }
        return OkValue();
    }

    E UnwrapErr() const {
        if (IsOk()) { throw std::runtime_error("Called UnwrapErr on an success value"); }
        return ErrValue();
    }

    T UnwrapOr(const T& defaultValue) const {
        if (IsOk()) { return OkValue(); }
        return defaultValue;
    }

    template<typename F>
    auto Map(F&& f) const -> Result<decltype(f(OkValue())), E> {
        if (IsOk()) { return Ok(f(std::get<T>(result))); }
        return Err(std::get<E>(result));
    }

    template<typename F>
    auto MapErr(F&& f) const -> Result<T, decltype(f(std::declval<E>()))> {
        if (IsErr()) { return Err(f(std::get<E>(result))); }
        return Ok(std::get<T>(result));
    }

    template<typename OkFunc, typename ErrFunc>
    auto Match(OkFunc&& okFn, ErrFunc&& errFn) const -> decltype(okFn(std::declval<T>())) {
        if (IsOk()) { return okFn(std::get<T>(result)); }
        return errFn(std::get<E>(result));
    }
};