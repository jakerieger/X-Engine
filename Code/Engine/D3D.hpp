#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

using namespace DirectX;

#include <comdef.h>
#include <stdexcept>

#ifndef PLATFORM_INCLUDED
    #include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif