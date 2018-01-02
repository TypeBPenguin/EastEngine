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
			struct Data
			{
				std::unique_ptr<const char[]> pString;
				std::size_t nLength = 0;

				Data(std::unique_ptr<const char[]> pString, std::size_t nLength)
					: pString(std::move(pString))
					, nLength(nLength)
				{
				}
			};

			std::unordered_map<StringKey, Data> umapStringTable;
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

		inline const StringTable::Data* GetStringPtr(const StringKey& key)
		{
			auto iter = s_pStringTable->umapStringTable.find(key);
			if (iter != s_pStringTable->umapStringTable.end())
				return &iter->second;

			return nullptr;
		}

		inline void SetStringPtr(const StringKey& key, std::unique_ptr<const char[]> str, std::size_t nLength)
		{
			s_pStringTable->umapStringTable.emplace(key, StringTable::Data(std::move(str), nLength));
		}

		bool Init()
		{ 
			if (s_pStringTable != nullptr)
			{
				LOG_MESSAGE("StringTable is Already Init");
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

			StringKey hashKey(Hash(str));

			if (hashKey == UnregisteredKey)
				return UnregisteredKey;

			const StringTable::Data* pExistStringData = GetStringPtr(hashKey);
			if (pExistStringData != nullptr)
			{
			#if CHECK_DUPLICATE_STRING_KEY != 0 || defined(DEBUG) || defined(_DEBUG)
				if (String::IsEquals(str, pExistStringData->pString.get()) == false)
				{
					LOG_ERROR("String Hash Crash, Duplicate String Key!! Hash : %u / %s != %s", hashKey, str, pExistStringData->pString.get());
				}
			#endif

				return hashKey;
			}

			std::size_t nLength = String::Length(str);
			std::unique_ptr<char[]> pNewStr(new char[nLength + 1]);
			String::Copy(pNewStr.get(), nLength + 1, str);

			SetStringPtr(hashKey, std::move(pNewStr), nLength);

			++s_pStringTable->nRegisteredKeyCount;

			return hashKey;
		}

		const char* GetString(const StringKey& key, std::size_t& nLength_out)
		{
			const StringTable::Data* pStringData = GetStringPtr(key);
			if (pStringData != nullptr)
			{
				nLength_out = pStringData->nLength;
				return pStringData->pString.get();
			}

			nLength_out = 0;
			return StrID::Unregistered.c_str();
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