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
			StringID(const std::string& str);
			StringID(const std::wstring& str);
			StringID(std::wstring&& str);
			~StringID() = default;

			const wchar_t* c_str() const noexcept { return m_pString->c_str(); }
			size_t length() const noexcept { return m_pString->size(); }
			bool empty() const noexcept { return m_pString->empty(); }

			const std::wstring* GetData() const { return m_pString; }
			operator const std::wstring& () const { return *m_pString; }
			const std::wstring& operator*() const { return *m_pString; }
			const std::wstring* operator->() const { return m_pString; }

			bool operator==(const StringID& rhs) const { return m_pString == rhs.m_pString; }
			bool operator!=(const StringID& rhs) const { return m_pString != rhs.m_pString; }
			bool operator==(const wchar_t* rhs) const { return *m_pString == rhs; }
			bool operator!=(const wchar_t* rhs) const { return *m_pString != rhs; }

			int compare(const StringID& other) const { return m_pString->compare(*other); }
			auto compare_fast(const StringID& other) const { return m_pString - other.m_pString; }

			bool less(const StringID& other) const { return *m_pString < *other; }
			bool less_fast(const StringID& other) const { return m_pString < other.m_pString; }

		private:
			const std::wstring* m_pString{ nullptr };
		};

		struct StringID_Less
		{
			bool operator()(const StringID& a, const StringID& b) const { return a.less(b); }
		};

		struct StringID_LessFast
		{
			bool operator()(const StringID& a, const StringID& b) const { return a.less_fast(b); }
		};

#define RegisterStringID(name)	static const est::string::StringID name(L#name);
	};
}

namespace sid
{
	extern const est::string::StringID None;
}

namespace std
{
	template <>
	struct hash<est::string::StringID>
	{
		const size_t operator()(const est::string::StringID& key) const
		{
			return reinterpret_cast<size_t>(key.GetData());
		}
	};
}