#pragma once

#include "StringUtil.h"

namespace eastengine
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
			StringID(const StringID& source);
			~StringID();

			bool operator == (const StringID& rValue) const { return m_pStringData == rValue.m_pStringData; }
			bool operator == (const char* rValue) const;

			bool operator != (const StringID& rValue) const { return m_pStringData != rValue.m_pStringData; }
			bool operator != (const char* rValue) const;

			const char* c_str() const;
			size_t length() const;

			const StringData* Key() const { return m_pStringData; }

			bool empty() const;

			void clear();

			StringID& Format(const char* format, ...);

		private:
			const StringData* m_pStringData{ nullptr };
		};
		
#define RegisterStringID(name)	static const eastengine::string::StringID name(#name);
	};
}

namespace StrID
{
	static const eastengine::string::StringID EmptyString("");
}

namespace std
{
	template <>
	struct hash<eastengine::string::StringID>
	{
		const eastengine::string::StringData* operator()(const eastengine::string::StringID& key) const
		{
			return key.Key();
		}
	};
}