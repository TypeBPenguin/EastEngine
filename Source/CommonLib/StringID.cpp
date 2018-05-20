#include "stdafx.h"
#include "StringID.h"

#include "StringTable.h"

namespace eastengine
{
	namespace String
	{
		StringID::StringID()
			: m_nStringKey(UnregisteredKey)
		{
			m_strPtr = String::GetString(m_nStringKey, &m_nLength);
		}

		StringID::StringID(const char* str)
			: m_nStringKey(String::Register(str))
		{
			m_strPtr = String::GetString(m_nStringKey, &m_nLength);
		}

		StringID::StringID(const StringKey& key)
			: m_nStringKey(key)
		{
			m_strPtr = String::GetString(m_nStringKey, &m_nLength);
		}

		StringID::StringID(const StringID& source)
			: m_nStringKey(source.m_nStringKey)
			, m_strPtr(source.m_strPtr)
			, m_nLength(source.m_nLength)
		{
		}

		StringID::~StringID()
		{
		}

		StringID& StringID::Format(const char* format, ...)
		{
			va_list args;
			va_start(args, format);
			std::size_t size = std::vsnprintf(nullptr, 0, format, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
			va_start(args, format);
			std::vsnprintf(buf.get(), size, format, args);
			va_end(args);

			m_nStringKey = String::GetKey(buf.get());
			m_strPtr = String::GetString(m_nStringKey, &m_nLength);

			return *this;
		}
	}
}