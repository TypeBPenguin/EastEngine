#pragma once

#include "CommonLib/ErrorDefine.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include "CommonLib/CommonLib.h"

#include <array>
#include <queue>
#include <unordered_map>
#include <ppl.h>

#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <D3Dcommon.h>

#include "ExternLib/DirectXTex/DirectXTex.h"