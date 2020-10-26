#pragma once

#include "CommonLib/ErrorDefine.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "CommonLib/CommonLib.h"

#include <concurrent_queue.h>

#define STB_IMAGE_IMPLEMENTATION
#include "ExternLib/stb-master/stb_image.h"