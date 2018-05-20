#pragma once

#include "PhantomType.h"

#ifndef CHECK_DUPLICATE_STRING_KEY
#define CHECK_DUPLICATE_STRING_KEY 0
#endif

namespace eastengine
{
	namespace String
	{
		struct tStringKey {};
		using StringKey = PhantomType<tStringKey, std::uint64_t>;
		
		static const StringKey UnregisteredKey(1);	// '\0'

		constexpr std::uint64_t Hash(const char* pString)
		{
			std::uint64_t v = 1;
			while (char c = *pString++)
			{
				v = (v << 6) + (v << 16) - v + c;
			}

			return v;
		}

		class StringID
		{
		public:
			StringID();
			StringID(const char* str);
			StringID(const StringKey& key);
			StringID(const StringID& source);
			~StringID();

			bool operator == (const StringID& rValue) const { return m_nStringKey == rValue.Key(); }
			bool operator == (const StringID& rValue) { return m_nStringKey == rValue.Key(); }
			bool operator == (const char* rValue) const { if (rValue == nullptr) return false; return m_nStringKey == StringID(rValue).Key(); }
			bool operator == (const char* rValue) { if (rValue == nullptr) return false; return m_nStringKey == StringID(rValue).Key(); }
			bool operator == (StringKey rValue) const { return m_nStringKey == rValue; }
			bool operator == (StringKey rValue) { return m_nStringKey == rValue; }

			bool operator != (const StringID& rValue) const { return m_nStringKey != rValue.Key(); }
			bool operator != (const StringID& rValue) { return m_nStringKey != rValue.Key(); }
			bool operator != (const char* rValue) const { if (rValue == nullptr) return true; return m_nStringKey != StringID(rValue).Key(); }
			bool operator != (const char* rValue) { if (rValue == nullptr) return true; return m_nStringKey != StringID(rValue).Key(); }
			bool operator != (StringKey rValue) const { return m_nStringKey != rValue; }
			bool operator != (StringKey rValue) { return m_nStringKey != rValue; }

			const char* c_str() const
			{
				if (m_strPtr == nullptr)
					return "";

				return m_strPtr;
			}

			std::size_t length() const { return m_nLength; }

			StringKey const Key() const { return m_nStringKey; }

			bool empty() const { return m_nStringKey == UnregisteredKey; }

			void clear() { m_nStringKey = UnregisteredKey; }

			StringID& Format(const char* format, ...);

		private:
			const char* m_strPtr;
			std::size_t m_nLength;
			StringKey m_nStringKey;
		};
	}

	#define RegisterStringID(name)	static const eastengine::String::StringID name(#name);
}

constexpr std::uint64_t operator "" _s(const char* str, std::size_t)
{
	return eastengine::String::Hash(str);
}

namespace StrID
{
	static const eastengine::String::StringID Unregistered("");
}

namespace std
{
	template <>
	struct hash<eastengine::String::StringKey>
	{
		std::uint64_t operator()(const eastengine::String::StringKey& key) const
		{
			return key.value;
		}
	};

	template <>
	struct hash<eastengine::String::StringID>
	{
		std::uint64_t operator()(const eastengine::String::StringID& key) const
		{
			return key.Key().value;
		}
	};
}