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
			boost::unordered_map<StringKey, char*> umapStringTable;
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
				for (auto& iter : umapStringTable)
				{
					if (iter.second != nullptr)
					{
						delete[] iter.second;
					}
				}
				umapStringTable.clear();
			}
		};
		static std::shared_ptr<StringTable> s_pStringTable = nullptr;

		inline StringKey Hash(const char* pString)
		{
			StringKey v = 1;
			while (char c = *pString++)
			{
				v = (v << 6) + (v << 16) - v + c;
			}

			return v;
		}

		inline char* GetStringPtr(StringKey key) { return s_pStringTable->umapStringTable[key]; }
		inline void SetStringPtr(StringKey key, char* str) { s_pStringTable->umapStringTable[key] = str; }

		bool Init()
		{
			if (s_pStringTable != nullptr)
				return true;

			s_pStringTable = std::make_shared<StringTable>();
			s_pStringTable->Init();

			return true;
		}

		void Release()
		{
			if (s_pStringTable == nullptr)
				return;

			s_pStringTable->Release();
			s_pStringTable.reset();
		}

		StringKey Register(const char* str)
		{
			if (s_pStringTable == nullptr)
			{
				Init();
			}

			std::unique_lock<std::mutex> lock(s_pStringTable->mutex);

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
			char* pNewStr = new char[nLength];
			String::Copy(pNewStr, nLength, str);

			SetStringPtr(hashKey, pNewStr);

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