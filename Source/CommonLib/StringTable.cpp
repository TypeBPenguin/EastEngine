#include "stdafx.h"
#include "StringTable.h"

#include "Lock.h"
#include "Log.h"

namespace sid
{
	const est::string::StringID EmptyString(L"");
	const est::string::StringID None(L"None");
}

namespace est
{
	namespace string
	{
		struct StringData
		{
			wchar_t* pString{ nullptr };
			size_t length{ 0 };

			~StringData()
			{
				free(pString);
			}

			std::unique_ptr<StringData> pNext;
		};

		struct StringTable
		{
			static const size_t TABLE_SIZE{ 4096 * 512 };

			std::vector<std::unique_ptr<StringData>> tables{ TABLE_SIZE };
			size_t count{ 0 };

			thread::SRWLock srwLock;

			size_t GetCount() const { return count; }

			const StringData* Register(const wchar_t* str, size_t length)
			{
				if (str == nullptr)
					return sid::EmptyString.Key();

				const uint64_t key = Hash(str);

				thread::SRWWriteLock writeLock(&srwLock);
				{
					const StringData* pStringData = tables[key % TABLE_SIZE].get();
					if (pStringData != nullptr)
					{
						while (pStringData != nullptr)
						{
							if (pStringData->length == length && string::IsEquals(pStringData->pString, str) == true)
								return pStringData;

							pStringData = pStringData->pNext.get();
						}
					}
				}

				std::unique_ptr<StringData> pNewStringData = std::make_unique<StringData>();
				pNewStringData->pString = static_cast<wchar_t*>(malloc(sizeof(wchar_t) * (length + 1)));
				string::Copy(pNewStringData->pString, length + 1, str);

				pNewStringData->length = length;

				pNewStringData->pNext = std::move(tables[key % TABLE_SIZE]);
				tables[key % TABLE_SIZE] = std::move(pNewStringData);

				++count;
				return tables[key % TABLE_SIZE].get();
			}

			const StringData* Register(const wchar_t* str)
			{
				if (str == nullptr)
					return sid::EmptyString.Key();

				const size_t length = string::Length(str);

				return Register(str, length);
			}

			uint64_t Hash(const wchar_t* pString) const
			{
				uint64_t v = 1;
				while (wchar_t c = *pString++)
				{
					v = (v << 6) + (v << 16) - v + c;
				}
				return v;
			}
		};

		// StringTable 을 shared_ptr 또는 unique_ptr로 관리하게 될 경우,
		// exe 프로젝트에서 한번, 라이브러리 프로젝트에서 한번
		// 총 2번 초기화 하는 현상이 발생하게 되어, 메모리릭이 생기게 됨
		static StringTable* s_pStringTable = nullptr;

		bool Init()
		{ 
			if (s_pStringTable != nullptr)
			{
				LOG_WARNING(L"StringTable is Already Init");
				return true;
			}

			s_pStringTable = new StringTable;

			return true;
		}

		void Release()
		{
			if (s_pStringTable == nullptr)
				return;

			delete s_pStringTable;
			s_pStringTable = nullptr;
		}

		const StringData* Register(const wchar_t* str, size_t length)
		{
			if (s_pStringTable == nullptr)
			{
				Init();
			}
			return s_pStringTable->Register(str, length);
		}

		const StringData* Register(const char* str)
		{
			if (s_pStringTable == nullptr)
			{
				Init();
			}

			const std::wstring wideString = string::MultiToWide(str);
			return s_pStringTable->Register(wideString.c_str());
		}

		const StringData* Register(const wchar_t* str)
		{
			if (s_pStringTable == nullptr)
			{
				Init();
			}
			return s_pStringTable->Register(str);
		}

		size_t GetRegisteredStringCount()
		{
			if (s_pStringTable == nullptr)
				return 0;

			return s_pStringTable->count;
		}

		StringID::StringID()
		{
		}

		StringID::StringID(const char* str)
			: m_pStringData(Register(str))
		{
		}

		StringID::StringID(const wchar_t* str)
			: m_pStringData(Register(str))
		{
		}

		StringID::StringID(const StringID& source)
			: m_pStringData(source.m_pStringData)
		{
		}

		StringID::StringID(StringID&& source) noexcept
			: m_pStringData(std::move(source.m_pStringData))
		{
		}

		StringID::~StringID()
		{
		}

		StringID& StringID::operator = (const StringID& source)
		{
			m_pStringData = source.m_pStringData;
			return *this;
		}

		StringID& StringID::operator = (StringID&& source) noexcept
		{
			m_pStringData = std::move(source.m_pStringData);
			return *this;
		}

		bool StringID::operator == (const wchar_t* rValue) const
		{
			return string::IsEquals(c_str(), rValue);
		}

		bool StringID::operator != (const wchar_t* rValue) const
		{
			return string::IsEquals(c_str(), rValue) == false;
		}

		const wchar_t* StringID::c_str() const
		{
			if (m_pStringData == nullptr)
				return L"";

			return m_pStringData->pString;
		}

		size_t StringID::length() const
		{
			if (m_pStringData == nullptr)
				return 0;

			return m_pStringData->length;
		}

		bool StringID::empty() const
		{
			if (m_pStringData == nullptr)
				return true;

			return m_pStringData->length == 0;
		}

		void StringID::clear()
		{
			m_pStringData = nullptr;
		}

		StringID& StringID::Format(const wchar_t* format, ...)
		{
			va_list args;
			va_start(args, format);
			uint32_t size = std::vswprintf(nullptr, 0, format, args) + 1;
			va_end(args);

			std::unique_ptr<wchar_t[]> buf = std::make_unique<wchar_t[]>(size);
			va_start(args, format);
			std::vswprintf(buf.get(), size, format, args);
			va_end(args);

			m_pStringData = Register(buf.get(), size);

			return *this;
		}
	};
}