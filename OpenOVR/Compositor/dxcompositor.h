#pragma once

// TODO Turtle1331 should this be here or distributed among dx*compositor.h?
#include "compositor.h"

// DirectX headers for use in dx*compositor.cpp

#ifdef SUPPORT_DX
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d12.h>

// Windows template libraries
#include <atlbase.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#endif
