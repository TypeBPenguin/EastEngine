#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include <assert.h>
#include <Windows.h>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <array>
#include <vector>
#include <queue>
#include <stack>
#include <variant>
#include <optional>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <future>
#include <atomic>

#include "robin_map.h"
#include "robin_set.h"

inline void SetBitMask(int& mask, int nBit)
{
	mask |= (1 << nBit);
}

inline bool GetBitMask(int mask, int nBit)
{
	return (mask & (1 << nBit)) != 0;
}

inline void SetBitMask64(long long& mask, int nBit)
{
	mask |= (1ui64 << nBit);
}

inline void SetBitMask64(unsigned long long& mask, int nBit)
{
	mask |= (1ui64 << nBit);
}

inline bool GetBitMask64(long long mask, int nBit)
{
	return (mask & (1ui64 << nBit)) != 0;
}

inline bool GetBitMask64(unsigned long long mask, int nBit)
{
	return (mask & (1ui64 << nBit)) != 0;
}

template <typename enumtype, typename storagetype = uint32_t>
class EnumFlags
{
public:
	EnumFlags()
	{
	}

	EnumFlags(enumtype e)
		: m_bits(static_cast<storagetype>(e))
	{
	}

	EnumFlags(const EnumFlags<enumtype, storagetype>& source)
		: m_bits(source.m_bits)
	{
	}

	explicit EnumFlags(storagetype b)
		: m_bits(b)
	{
	}

	bool operator==(enumtype e) const { return m_bits == static_cast<storagetype>(e); }
	bool operator==(const EnumFlags<enumtype, storagetype>& f) const { return m_bits == f.m_bits; }
	bool operator==(bool b) const { return bool(*this) == b; }
	bool operator!=(enumtype e) const { return m_bits != static_cast<storagetype>(e); }
	bool operator!=(const EnumFlags<enumtype, storagetype>& f) const { return m_bits != f.m_bits; }

	EnumFlags<enumtype, storagetype>& operator=(const EnumFlags<enumtype, storagetype>& f) { m_bits = f.m_bits; return *this; }
	EnumFlags<enumtype, storagetype>& operator=(enumtype e) { m_bits = static_cast<storagetype>(e); return *this; }

	EnumFlags<enumtype, storagetype>& operator|=(enumtype e) { m_bits |= static_cast<storagetype>(e); return *this; }
	EnumFlags<enumtype, storagetype>& operator|=(const EnumFlags<enumtype, storagetype>& f) { m_bits = f.m_bits; return *this; }
	EnumFlags<enumtype, storagetype> operator|(enumtype e) const { EnumFlags<enumtype, storagetype> out(*this); out |= e; return out; }
	EnumFlags<enumtype, storagetype> operator|(const EnumFlags<enumtype, storagetype>& f) const { m_bits |= f.m_bits; return *this; }

	EnumFlags<enumtype, storagetype>& operator&=(enumtype e) { m_bits &= static_cast<storagetype>(e); return *this; }
	EnumFlags<enumtype, storagetype>& operator&=(const EnumFlags<enumtype, storagetype>& f) { m_bits &= f.m_bits; return *this; }
	EnumFlags<enumtype, storagetype> operator&(enumtype e) const { EnumFlags<enumtype, storagetype> out = *this; out.m_bits &= static_cast<storagetype>(e); return out; }
	EnumFlags<enumtype, storagetype> operator&(const EnumFlags<enumtype, storagetype>& f) const { EnumFlags<enumtype, storagetype> out = *this; out.m_bits &= f.m_bits; return out; }

	EnumFlags<enumtype, storagetype>& operator^=(enumtype e) { m_bits ^= static_cast<storagetype>(e); return *this; }
	EnumFlags<enumtype, storagetype>& operator^=(const EnumFlags<enumtype, storagetype>& f) { m_bits ^= f.m_bits; return *this; }
	EnumFlags<enumtype, storagetype> operator^(enumtype e) const { EnumFlags<enumtype, storagetype> out = *this; out.m_bits ^= static_cast<storagetype>(e); return out; }
	EnumFlags<enumtype, storagetype> operator^(const EnumFlags<enumtype, storagetype>& f) const { EnumFlags<enumtype, storagetype> out = *this; out.m_bits ^= f.m_bits; return out; }

	EnumFlags<enumtype, storagetype> operator~() const { EnumFlags<enumtype, storagetype> out; out.m_bits = storagetype(~m_bits); return out; }

	operator bool() const { return m_bits ? true : false; }
	operator uint8_t() const { return static_cast<uint8_t>(m_bits); }
	operator uint16_t() const { return static_cast<uint16_t>(m_bits); }
	operator uint32_t() const { return static_cast<uint32_t>(m_bits); }
	operator uint64_t() const { return static_cast<uint64_t>(m_bits); }

	bool IsSet(enumtype e) const { return (m_bits & static_cast<storagetype>(e)) == static_cast<storagetype>(e); }
	EnumFlags<enumtype, storagetype>& Set(enumtype e) { m_bits = static_cast<storagetype>(e); }

	void Clear(enumtype e) { m_bits &= ~static_cast<storagetype>(e); }

public:
	friend EnumFlags<enumtype, storagetype> operator&(enumtype a, EnumFlags<enumtype, storagetype>& b)
	{
		EnumFlags<enumtype, storagetype> out;
		out.m_bits = a & b.m_bits;
		return out;
	}

private:
	storagetype m_bits{ 0 };
};

#define FLAGS_OPERATORS(enumtype, storagetype)									\
	inline EnumFlags<enumtype, storagetype> operator|(enumtype a, enumtype b)	\
	{																			\
		EnumFlags<enumtype, storagetype> r(a);									\
		r |= b;																	\
		return r;																\
	}																			\
	inline EnumFlags<enumtype, storagetype> operator&(enumtype a, enumtype b)	\
	{																			\
		EnumFlags<enumtype, storagetype> r(a);									\
		r &= b;																	\
		return r;																\
	}																			\
	inline EnumFlags<enumtype, storagetype> operator~(enumtype a)				\
	{																			\
		return ~EnumFlags<enumtype, storagetype>(a);							\
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
	std::string m_message;
};
#define throw_line(expression) throw my_exception(expression, __FILE__, __LINE__);

template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

#include "Memory.h"
#include "Math.h"
#include "Collision.h"
#include "StringTable.h"
#include "Log.h"
#include "Performance.h"
#include "JobSystem.h"