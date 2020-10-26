﻿#pragma once

#include "CommonLib/ErrorDefine.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include "CommonLib/CommonLib.h"

#include <WinSock2.h>
#include <WS2tcpip.h>