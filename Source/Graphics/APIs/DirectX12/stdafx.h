#pragma once

#include "CommonLib/ErrorDefine.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include "CommonLib/CommonLib.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>

#pragma warning(push)
#pragma warning(disable:4324)
#include "d3dx12.h"
#pragma warning(pop)

#include "ExternLib/DirectX/DirectXTex/DirectXTex.h"