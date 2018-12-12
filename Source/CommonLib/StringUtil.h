#pragma once

namespace eastengine
{
	namespace string
	{
		inline size_t Length(const char* string) { return strlen(string); }

		inline bool Copy(char* strDestination, size_t size, const char* strSource, size_t nMaxCount = _TRUNCATE)
		{
			if (strDestination == nullptr || strSource == nullptr)
			{
				assert(false);
				return false;
			}

			if (nMaxCount != _TRUNCATE && size <= nMaxCount)
			{
				assert(false);
				nMaxCount = _TRUNCATE;
			}

			strncpy_s(strDestination, size, strSource, nMaxCount);

			return true;
		}

		template <size_t size>
		inline bool Copy(char(&strDestination)[size], const char* strSource, size_t nMaxCount = _TRUNCATE)
		{
			return Copy(strDestination, size, strSource, nMaxCount);
		}

		inline bool Copy(wchar_t* strDestination, size_t size, const wchar_t* strSource, size_t nMaxCount = _TRUNCATE)
		{
			if (strDestination == nullptr || strSource == nullptr)
			{
				assert(false);
				return false;
			}

			if (nMaxCount != _TRUNCATE && size <= nMaxCount)
			{
				assert(false);
				nMaxCount = _TRUNCATE;
			}

			wcsncpy_s(strDestination, size, strSource, nMaxCount);

			return true;
		}

		template <size_t size>
		inline bool Copy(wchar_t(&strDestination)[size], const wchar_t* strSource, size_t nMaxCount = _TRUNCATE)
		{
			return Copy(strDestination, size, strSource, nMaxCount);
		}

		inline bool Concat(char* strDestination, size_t size, const char* strSource, size_t nMaxCount = _TRUNCATE)
		{
			if (strDestination == nullptr || strSource == nullptr)
			{
				assert(false);
				return false;
			}

			if (nMaxCount != _TRUNCATE && size <= nMaxCount)
			{
				assert(false);
				nMaxCount = _TRUNCATE;
			}

			strncat_s(strDestination, size, strSource, nMaxCount);

			return true;
		}

		template <size_t size>
		inline bool Concat(char(&strDestination)[size], const char* strSource, size_t nMaxCount = _TRUNCATE)
		{
			return Concat(strDestination, size, strSource, nMaxCount);
		}

		inline bool Concat(wchar_t* strDestination, size_t size, const wchar_t* strSource, size_t nMaxCount = _TRUNCATE)
		{
			if (strDestination == nullptr || strSource == nullptr)
			{
				assert(false);
				return false;
			}

			if (nMaxCount != _TRUNCATE && size <= nMaxCount)
			{
				assert(false);
				nMaxCount = _TRUNCATE;
			}

			wcsncat_s(strDestination, size, strSource, nMaxCount);

			return true;
		}

		template <size_t size>
		inline bool Concat(wchar_t(&strDestination)[size], const wchar_t* strSource, size_t nMaxCount = _TRUNCATE)
		{
			return Concat(strDestination, size, strSource, nMaxCount);
		}
		
		inline bool IsEquals(const char* str1, const char* str2, int* returnValue = nullptr)
		{
			if (str1 == nullptr || str2 == nullptr)
			{
				assert(false);
				*returnValue = 0;
				return false;
			}

			if (returnValue != nullptr)
			{
				*returnValue = strcmp(str1, str2);
				return *returnValue == 0;
			}
			else
			{
				return strcmp(str1, str2) == 0;
			}
		}

		inline bool IsEquals(const wchar_t* str1, const wchar_t* str2, int* returnValue = nullptr)
		{
			if (str1 == nullptr || str2 == nullptr)
			{
				assert(false);
				*returnValue = 0;
				return false;
			}

			if (returnValue != nullptr)
			{
				*returnValue = wcscmp(str1, str2);
				return *returnValue == 0;
			}
			else
			{
				return wcscmp(str1, str2) == 0;
			}
		}

		inline bool IsEqualsNoCase(const char* str1, const char* str2, int* returnValue = nullptr)
		{
			if (str1 == nullptr || str2 == nullptr)
			{
				assert(false);
				*returnValue = 0;
				return false;
			}

			if (returnValue != nullptr)
			{
				*returnValue = _stricmp(str1, str2);
				return *returnValue == 0;
			}
			else
			{
				return _stricmp(str1, str2) == 0;
			}
		}

		inline bool IsEqualsNoCase(const wchar_t* str1, const wchar_t* str2, int* returnValue = nullptr)
		{
			if (str1 == nullptr || str2 == nullptr)
			{
				assert(false);
				*returnValue = 0;
				return false;
			}

			if (returnValue != nullptr)
			{
				*returnValue = _wcsicmp(str1, str2);
				return *returnValue == 0;
			}
			else
			{
				return _wcsicmp(str1, str2) == 0;
			}
		}

		std::vector<std::string> Tokenizer(const char* string, const char* strDelimiter);
		std::vector<std::string> Tokenizer(const std::string& string, const char* strDelimiter);

		char* ToUpper(char* string, uint32_t nLength);
		template <typename T, size_t size>
		char* ToUpper(char(&string)[size]) { return ToUpper(string, size); }

		char* ToLower(char* string, uint32_t nLength);
		template <typename T, size_t size>
		char* ToLower(char(&string)[size]) { return ToLower(string, size); }

		template <typename T>
		std::string ToString(T value) { return std::to_string(value); }

		template <typename T>
		T ToValue(const char* string);

		std::string Format(const char* format, ...);
		std::string RandomString(uint32_t nLength, bool isOnlyAlphabet = true);

		std::string WideToMulti(const wchar_t* strWide, DWORD dwCodePage = CP_ACP);
		std::string WideToMulti(const std::wstring& strWide, DWORD dwCodePage = CP_ACP);
		std::wstring MultiToWide(const char* strMulti, DWORD dwCodePage = CP_ACP);
		std::wstring MultiToWide(const std::string& strMulti, DWORD dwCodePage = CP_ACP);

		std::string Utf8ToMulti(const wchar_t* strWide, DWORD dwCodePage = CP_UTF8);
		std::string Utf8ToMulti(const std::wstring& strWide, DWORD dwCodePage = CP_UTF8);
		std::wstring MultiToUtf8(const char* strMulti, DWORD dwCodePage = CP_UTF8);
		std::wstring MultiToUtf8(const std::string& strMulti, DWORD dwCodePage = CP_UTF8);
	};
}