// Author: Jake Rieger
// Created: 1/16/2025.
//
#pragma once

#include "Types.hpp"
#include <locale>
#include <codecvt>

namespace x {
    inline str WideToAnsi(const wstr& input) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        try {
            return converter.to_bytes(input);
        } catch (const std::range_error&) { return str(); }
    }

    inline wstr AnsiToWide(const str& input) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        try {
            return converter.from_bytes(input);
        } catch (const std::range_error&) { return wstr(); }
    }
}