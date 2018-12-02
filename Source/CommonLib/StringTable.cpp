#include "stdafx.h"
#include "StringTable.h"

#include "Lock.h"
#include "Log.h"

namespace StrID
{
	const eastengine::string::StringID EmptyString("");
	const eastengine::string::StringID None("None");
}

namespace eastengine
{
	namespace string
	{
		struct StringData
		{
			const char* pString{ nullptr };
			size_t nLength = 0;

			StringData* pNext{ nullptr };

			~StringData()
			{
				delete[] pString;
			}
		};

		struct StringTable
		{
			static const size_t TABLE_SIZE{ 4096 * 512 };

			std::vector<StringData*> tables;
			size_t nCount{ 0 };

			thread::SRWLock srwLock;

			StringTable()
			{
				tables.resize(TABLE_SIZE);
			}

			~StringTable()
			{
				for (size_t i = 0; i < TABLE_SIZE; ++i)
				{
					StringData* pData = tables[i];

					while (pData != nullptr)
					{
						StringData* pNext = pData->pNext;

						if (pData != nullptr)
						{
							delete pData;
							pData = nullptr;
						}

						pData = pNext;
					}
				}
				tables.clear();
			}

			size_t GetCount() const { return nCount; }

			const StringData* Register(const char* str, size_t nLength)
			{
				if (str == nullptr)
					return StrID::EmptyString.Key();

				const uint64_t key = Hash(str);

				thread::SRWWriteLock writeLock(&srwLock);

				const StringData* pStringData = tables[key % TABLE_SIZE];
				if (pStringData != nullptr)
				{
					while (pStringData != nullptr)
					{
						if (pStringData->nLength == nLength && string::IsEquals(pStringData->pString, str) == true)
							return pStringData;

						pStringData = pStringData->pNext;
					}
				}

				StringData* pNewStringData = new StringData;
				char* pNewString = new char[nLength + 1];
				string::Copy(pNewString, nLength + 1, str);

				pNewStringData->pString = pNewString;
				pNewStringData->nLength = nLength;

				pNewStringData->pNext = tables[key % TABLE_SIZE];
				tables[key % TABLE_SIZE] = pNewStringData;

				++nCount;
				return pNewStringData;
			}

			const StringData* Register(const char* str)
			{
				if (str == nullptr)
					return StrID::EmptyString.Key();

				const size_t nLength = string::Length(str);

				return Register(str, nLength);
			}

			uint64_t Hash(const char* pString) const
			{
				uint64_t v = 1;
				while (char c = *pString++)
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
				LOG_MESSAGE("StringTable is Already Init");
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

		const StringData* Register(const char* str, size_t nLength)
		{
			if (s_pStringTable == nullptr)
			{
				Init();
			}

			return s_pStringTable->Register(str, nLength);
		}

		const StringData* Register(const char* str)
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

			return s_pStringTable->nCount;
		}

		StringID::StringID()
		{
		}

		StringID::StringID(const char* str)
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

		bool StringID::operator == (const char* rValue) const
		{
			return string::IsEquals(c_str(), rValue);
		}

		bool StringID::operator != (const char* rValue) const
		{
			return string::IsEquals(c_str(), rValue) == false;
		}

		const char* StringID::c_str() const
		{
			if (m_pStringData == nullptr)
				return "";

			return m_pStringData->pString;
		}

		size_t StringID::length() const
		{
			if (m_pStringData == nullptr)
				return 0;

			return m_pStringData->nLength;
		}

		bool StringID::empty() const
		{
			if (m_pStringData == nullptr)
				return true;

			return m_pStringData->nLength == 0;
		}

		void StringID::clear()
		{
			m_pStringData = nullptr;
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

			m_pStringData = Register(buf.get(), size);

			return *this;
		}
	};
}