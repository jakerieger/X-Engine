// Author: Jake Rieger
// Created: 1/16/2025.
//
#pragma once

#include "Types.hpp"
#include "Memory.hpp"

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

    inline bool StrCopy(char* dst, const size_t dstSize, const char* src) {
        if (!dst || !src || dstSize == 0) { return false; }

        size_t srcLen = 0;
        for (; srcLen < dstSize && src[srcLen] != '\0'; ++srcLen) {}

        if (srcLen >= dstSize) {
            dst[0] = '\0';
            return false;
        }

        std::memcpy(dst, src, srcLen);
        dst[srcLen] = '\0';

        return true;
    }

    inline bool StrConcat(char* dst, const size_t dstSize, const char* src) {
        if (!dst || !src || dstSize == 0) { return false; }

        size_t dstLen = 0;
        for (; dstLen < dstSize && dst[dstLen] != '\0'; ++dstLen) {}

        if (dstLen >= dstSize) {
            dst[0] = '\0';
            return false;
        }

        const size_t spaceLeft = dstSize - dstLen;
        size_t srcLen          = 0;
        for (; srcLen < spaceLeft && src[srcLen] != '\0'; ++srcLen) {}

        if (srcLen >= spaceLeft) {
            return false; // Not enough space for concatenation
        }

        std::memcpy(dst + dstLen, src, srcLen);
        dst[dstLen + srcLen] = '\0';

        return true;
    }

    inline size_t StrLen(const char* str, const size_t maxLen) {
        if (!str) { return 0; }

        size_t len = 0;
        for (; len < maxLen && str[len] != '\0'; ++len) {}

        return len;
    }

    inline int StrCompare(const char* strA, const char* strB, const size_t maxLen) {
        if (!strA || !strB) { return strA ? 1 : (strB ? -1 : 0); }

        for (size_t i = 0; i < maxLen; ++i) {
            if (strA[i] != strB[i] || strA[i] == '\0') {
                return strA[i] - strB[i];
            }
        }

        return 0;
    }

    inline bool StrValidate(const char* str, const size_t maxLen) {
        if (!str) { return false; }

        bool foundNull = false;
        for (size_t i = 0; i < maxLen; ++i) {
            if (str[i] == '\0') {
                foundNull = true;
                break;
            }

            if (!isprint(CAST<u8>(str[i]))) {
                return false;
            }
        }

        return foundNull && (StrLen(str, maxLen) > 0);
    }
}