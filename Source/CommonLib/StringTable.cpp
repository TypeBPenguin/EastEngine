#include "stdafx.h"
#include "StringTable.h"

#include "Lock.h"
#include "Log.h"

namespace sid
{
	const est::string::StringID None(L"None");
}

namespace est
{
	namespace string
	{
		class Table
		{
		public:
			Table() = default;
			~Table() = default;

		public:
			const std::wstring& FindOrAdd(const char* str)
			{
				if (str == nullptr)
					return FindOrAdd(std::wstring{});

				return FindOrAdd(string::MultiToWide(str));
			}

			const std::wstring& FindOrAdd(const wchar_t* str)
			{
				if (str == nullptr)
					return FindOrAdd(std::wstring{});

				return FindOrAdd(std::wstring(str));
			}

			const std::wstring& FindOrAdd(const std::string& str)
			{
				return FindOrAdd(string::MultiToWide(str));
			}

			const std::wstring& FindOrAdd(std::wstring&& str)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);
				const auto result = m_set.insert(std::move(str));
				if (result.second)
				{
					m_memory += result.first->capacity() + 1;
				}
				return *result.first;
			}

			const std::wstring& FindOrAdd(const std::wstring& str)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);
				const auto result = m_set.insert(str);
				if (result.second)
				{
					m_memory += result.first->capacity() + 1;
				}
				return *result.first;
			}

			void Clear()
			{
				m_set.clear();
				m_memory = 0;
			}

			size_t GetUseCount() const { return m_set.size(); }
			size_t GetUseMemory() const { return m_memory; }

		private:
			thread::SRWLock m_srwLock;
			std::unordered_set<std::wstring> m_set;
			size_t m_memory{ 0 };
		};

		Table& GetTable()
		{
			static Table s_stringTable;
			return s_stringTable;
		}

		void Release()
		{
			GetTable().Clear();
		}

		static const std::wstring& GetDefaultString()
		{
			static const std::wstring& str = GetTable().FindOrAdd(std::wstring{});
			return str;
		}

		StringID::StringID()
			: m_pString{ &GetDefaultString() }
		{
		}

		StringID::StringID(const char* str)
			: m_pString(&GetTable().FindOrAdd(str))
		{
		}

		StringID::StringID(const wchar_t* str)
			: m_pString(&GetTable().FindOrAdd(str))
		{
		}

		StringID::StringID(const std::string& str)
			: m_pString(&GetTable().FindOrAdd(str))
		{
		}

		StringID::StringID(const std::wstring& str)
			: m_pString(&GetTable().FindOrAdd(str))
		{
		}

		StringID::StringID(std::wstring&& str)
			: m_pString(&GetTable().FindOrAdd(std::move(str)))
		{
		}
	};
}