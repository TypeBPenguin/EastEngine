#include "stdafx.h"
#include "StringTable.h"

#include "Log.h"

#define STRING_TABLE_SIZE (4096 * 1024)

namespace EastEngine
{
	namespace String
	{
		struct StringTable
		{
			boost::unordered_map<StringKey, std::unique_ptr<char[]>> umapStringTable;
			int nRegisteredKeyCount = 0;

			std::mutex mutex;

			~StringTable()
			{
				Release();
			}

			void Init()
			{
				umapStringTable.reserve(STRING_TABLE_SIZE);
				nRegisteredKeyCount = 0;
			}

			void Release()
			{
				umapStringTable.clear();
			}
		};

		// StringTable 을 shared_ptr 또는 unique_ptr로 관리하게 될 경우,
		// exe 프로젝트에서 한번, 라이브러리 프로젝트에서 한번
		// 총 2번 초기화 하는 현상이 발생하게 되어, 메모리릭이 생기게 됨
		static StringTable* s_pStringTable = nullptr;

		inline StringKey Hash(const char* pString)
		{
			std::size_t v = 1;
			while (char c = *pString++)
			{
				v = (v << 6) + (v << 16) - v + c;
			}

			return StringKey(v);
		}

		inline char* GetStringPtr(StringKey key) { return s_pStringTable->umapStringTable[key].get(); }
		inline void SetStringPtr(StringKey key, std::unique_ptr<char[]> str) { s_pStringTable->umapStringTable[key] = std::move(str); }

		bool Init()
		{ 
			if (s_pStringTable != nullptr)
			{
				PRINT_LOG("StringTable is Already Init");
				return true;
			}

			s_pStringTable = new StringTable;
			s_pStringTable->Init();

			return true;
		}

		void Release()
		{
			if (s_pStringTable == nullptr)
				return;

			s_pStringTable->Release();
			delete s_pStringTable;
			s_pStringTable = nullptr;
		}

		StringKey Register(const char* str)
		{
			if (s_pStringTable == nullptr)
			{
				Init();
			}

			std::lock_guard<std::mutex> lock(s_pStringTable->mutex);

			if (str == nullptr)
				return UnregisteredKey;

			StringKey hashKey = Hash(str);

			if (hashKey == UnregisteredKey)
				return UnregisteredKey;

			const char* strExistsKey = GetStringPtr(hashKey);
			if (strExistsKey != nullptr)
			{
			#if CHECK_DUPLICATE_STRING_KEY != 0 || defined(DEBUG) || defined(_DEBUG)
				if (String::IsEquals(str, strExistsKey) == false)
				{
					PRINT_LOG("String Hash Crash, Duplicate String Key!! Hash : %u / %s != %s", hashKey, str, strExistsKey);
				}
			#endif

				return hashKey;
			}

			std::size_t nLength = String::Length(str) + 1;
			std::unique_ptr<char[]> pNewStr(new char[nLength]);
			String::Copy(pNewStr.get(), nLength, str);

			SetStringPtr(hashKey, std::move(pNewStr));

			++s_pStringTable->nRegisteredKeyCount;

			return hashKey;
		}

		const char* GetString(StringKey key)
		{
			return GetStringPtr(key);
		}

		StringKey GetKey(const char* str)
		{
			return Register(str);
		}

		uint32_t GetRegisteredStringCount()
		{
			if (s_pStringTable == nullptr)
				return 0;

			return s_pStringTable->nRegisteredKeyCount;
		}
	};
}