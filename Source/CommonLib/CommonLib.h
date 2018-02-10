#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#ifndef IGNORE_DX_LIB
	#include <d3d11_1.h>
#endif

#include <assert.h>
#include <Windows.h>
#include <algorithm>
#include <cassert>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <variant>
#include <optional>
#include <numeric>
#include <array>
#include <string>
#include <future>

#include <ppl.h>
#include <concurrent_queue.h>

inline void SetBitMask(int& nMask, int nBit)
{
	nMask |= (1 << nBit);
}

inline bool GetBitMask(int nMask, int nBit)
{
	return (nMask & (1 << nBit)) != 0;
}

inline void SetBitMask64(long long& nMask, int nBit)
{
	nMask |= (1ui64 << nBit);
}

inline bool GetBitMask64(long long nMask, int nBit)
{
	return (nMask & (1ui64 << nBit)) != 0;
}

#define SafeDelete(ptr)	\
if (ptr != nullptr)		\
{						\
	delete ptr;			\
	ptr = nullptr;		\
}						\

#define SafeReleaseDelete(ptr)	\
if (ptr != nullptr)				\
{								\
	ptr->Release();				\
	delete ptr;					\
	ptr = nullptr;				\
}								\

#define SafeDeleteArray(ptr)	\
if (ptr != nullptr)				\
{								\
	delete[] ptr;				\
	ptr = nullptr;				\
}								\

#define SafeRelease(ptr)	\
if (ptr != nullptr)			\
{							\
	ptr->Release();			\
	ptr = nullptr;			\
}							\

struct DeleteSTLObject
{
	template<typename Pointer>
	void operator() (Pointer*& ptr) const
	{
		SafeDelete(ptr);
	}
};

struct ReleaseDeleteSTLObject
{
	template<typename Pointer>
	void operator() (Pointer*& ptr) const
	{
		SafeReleaseDelete(ptr);
	}
};

struct ReleaseSTLObject
{
	template<typename Pointer>
	void operator() (Pointer*& ptr) const
	{
		SafeRelease(ptr);
	}
};

struct DeleteSTLMapObject
{
	template<typename Key, typename Pointer>
	void operator() (std::pair<const Key, Pointer>& MapPair) const
	{
		SafeDelete(MapPair.second);
	}
};

struct ReleaseDeleteSTLMapObject
{
	template<typename Key, typename Pointer>
	void operator() (std::pair<const Key, Pointer>& MapPair) const
	{
		SafeReleaseDelete(MapPair.second);
	}
};

struct ReleaseSTLMapObject
{
	template<typename Key, typename Pointer>
	void operator() (std::pair<const Key, Pointer>& MapPair) const
	{
		SafeRelease(MapPair.second);
	}
};

#include "Memory.h"
#include "Math.h"
#include "Collision.h"
#include "StringTable.h"
#include "Log.h"
#include "Performance.h"