#pragma once

#include "StringUtil.h"

namespace est
{
	namespace string
	{
		void Release();

		struct StringData;

		class StringID
		{
		public:
			StringID();
			StringID(const char* str);
			StringID(const wchar_t* str);
			StringID(const StringID& source);
			StringID(StringID&& source) noexcept;
			~StringID();

			StringID& operator = (const StringID& source);
			StringID& operator = (StringID&& source) noexcept;

			bool operator == (const StringID& rValue) const { return m_pStringData == rValue.m_pStringData; }
			bool operator == (const wchar_t* rValue) const;

			bool operator != (const StringID& rValue) const { return m_pStringData != rValue.m_pStringData; }
			bool operator != (const wchar_t* rValue) const;

			const wchar_t* c_str() const;
			size_t length() const;

			const StringData* Key() const { return m_pStringData; }

			bool empty() const;

			void clear();

			StringID& Format(const wchar_t* format, ...);

		private:
			const StringData* m_pStringData{ nullptr };
		};
		
#define RegisterStringID(name)	static const est::string::StringID name(L#name);
	};
}

namespace sid
{
	extern const est::string::StringID EmptyString;
	extern const est::string::StringID None;
}

namespace std
{
	template <>
	struct hash<est::string::StringID>
	{
		const size_t operator()(const est::string::StringID& key) const
		{
			return reinterpret_cast<size_t>(key.Key());
		}
	};
}