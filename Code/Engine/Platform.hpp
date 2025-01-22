#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <windowsx.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#ifndef PLATFORM_INCLUDED
#define PLATFORM_INCLUDED
#endif