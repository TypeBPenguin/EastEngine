#pragma once

#include "StringUtil.h"
#include "StringID.h"

namespace EastEngine
{
	namespace String
	{
		void Release();

		StringKey Register(const char* str);

		const char* GetString(const StringKey& key, std::size_t& nLength_out);
		StringKey GetKey(const char* str);

		uint32_t GetRegisteredStringCount();
	};
}