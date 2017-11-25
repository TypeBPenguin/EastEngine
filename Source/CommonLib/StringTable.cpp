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

		// StringTable �� shared_ptr �Ǵ� unique_ptr�� �����ϰ� �� ���,
		// exe ������Ʈ���� �ѹ�, ���̺귯�� ������Ʈ���� �ѹ�
		// �� 2�� �ʱ�ȭ �ϴ� ������ �߻��ϰ� �Ǿ�, �޸𸮸��� ����� ��
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