// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "ErrorDefine.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include <assert.h>
#include <Windows.h>

#include <mutex>
#include <algorithm>
#include <string>

#include <random>
#include <chrono>

#include <vector>
#include <list>

#include <ppl.h>
#include <concurrent_queue.h>

#include <boost/unordered_map.hpp>