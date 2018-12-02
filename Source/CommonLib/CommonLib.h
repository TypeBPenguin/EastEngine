#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include <assert.h>
#include <Windows.h>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <array>
#include <queue>
#include <variant>

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <future>
#include <atomic>

#include "robin_map.h"
#include "robin_set.h"

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

inline void SetBitMask64(unsigned long long& nMask, int nBit)
{
	nMask |= (1ui64 << nBit);
}

inline bool GetBitMask64(long long nMask, int nBit)
{
	return (nMask & (1ui64 << nBit)) != 0;
}

inline bool GetBitMask64(unsigned long long nMask, int nBit)
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

class my_exception : public std::runtime_error
{
public:
	my_exception(const char* expression, const char* fileName, unsigned int lineNo);
	~my_exception() = default;

	const char* what() const throw();

private:
	std::string m_strMsg;
};
#define throw_line(expression) throw my_exception(expression, __FILE__, __LINE__);

#include "Memory.h"
#include "Math.h"
#include "Collision.h"
#include "StringTable.h"
#include "Log.h"
#include "Performance.h"