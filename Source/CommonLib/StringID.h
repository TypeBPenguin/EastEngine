#pragma once

#ifndef CHECK_DUPLICATE_STRING_KEY
#define CHECK_DUPLICATE_STRING_KEY 0
#endif

namespace EastEngine
{
	namespace String
	{
		typedef std::size_t StringKey;
		const StringKey UnregisteredKey = 1;	// '\0'

		class StringID
		{
		public:
			StringID();
			StringID(const char* str);
			StringID(StringKey key);
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

			StringKey const Key() const { return m_nStringKey; }

			bool empty() const { return m_nStringKey == UnregisteredKey; }

			void clear() { m_nStringKey = UnregisteredKey; }

			StringID& Format(const char* format, ...);

		private:
			StringKey m_nStringKey;
			const char* m_strPtr;
		};
	}

	#define RegisterStringID(name)	static const EastEngine::String::StringID name(#name);
}

namespace StrID
{
	static const EastEngine::String::StringID Unregistered("");
}

namespace std
{
	template <>
	struct hash<EastEngine::String::StringID>
	{
		std::size_t operator()(const EastEngine::String::StringID& key) const
		{
			return key.Key();
		}
	};
}

namespace boost
{
	template <>
	struct hash<EastEngine::String::StringID>
	{
		std::size_t operator()(const EastEngine::String::StringID& key) const
		{
			return key.Key();
		}
	};
}