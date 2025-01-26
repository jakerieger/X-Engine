#pragma once

#define NOMINMAX

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

using namespace DirectX;

#include <comdef.h>
#include <stdexcept>

#include "Common/Str.hpp" // for WideToAnsi()

#ifndef PLATFORM_INCLUDED
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

namespace x {
    class DxException final : public std::runtime_error {
        HRESULT _errCode;
        std::string _func;
        std::string _file;
        int _line;

    public:
        DxException(const HRESULT hr, const std::string& func, const std::string& file, const int line)
            : std::runtime_error(FormatErrMessage(hr, file, line, func)), _errCode(hr), _func(func),
              _file(file), _line(line) {}

        [[nodiscard]] HRESULT ErrCode() const {
            return _errCode;
        }

        [[nodiscard]] const std::string& FunctionName() const {
            return _func;
        }

        [[nodiscard]] const std::string& FileName() const {
            return _file;
        }

        [[nodiscard]] const int& LineNumber() const {
            return _line;
        }

    private:
        static std::string FormatErrMessage(const HRESULT hr,
                                            const std::string& file,
                                            int line,
                                            const std::string& func) {
            const _com_error err(hr);
            std::string errMsg = WideToAnsi(err.ErrorMessage());
            char buffer[1024];
            _i_ = snprintf(buffer,
                           1024,
                           "%s:%d D3D API Error:\n In: %s\n Message: %s\n",
                           file.c_str(),
                           line,
                           func.c_str(),
                           errMsg.c_str());
            return {buffer};
        }
    };
}

#define DX_THROW_IF_FAILED(hr)                                                                                         \
    if (FAILED(hr)) throw x::DxException(hr, __FUNCTION__, __FILE__, __LINE__);