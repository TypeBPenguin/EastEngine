#pragma once

#include "StringUtil.h"
#include "StringID.h"

namespace eastengine
{
	namespace String
	{
		void Release();

		StringKey Register(const char* str);

		const char* GetString(const StringKey& key, std::size_t* pLength_out = nullptr);
		StringKey GetKey(const char* str);

		size_t GetRegisteredStringCount();
	};
}