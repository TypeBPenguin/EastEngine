#pragma once

#include <string>

namespace est
{
	namespace string
	{
		inline size_t Length(const char* string) { return strlen(string); }
		inline size_t Length(const wchar_t* string) { return wcslen(string); }

		inline bool Copy(char* destination, size_t size, const char* source, size_t maxCount = _TRUNCATE)
		{
			if (destination == nullptr || source == nullptr)
			{
				assert(false);
				return false;
			}

			if (maxCount != _TRUNCATE && size <= maxCount)
			{
				assert(false);
				maxCount = _TRUNCATE;
			}

			strncpy_s(destination, size, source, maxCount);

			return true;
		}

		template <size_t size>
		inline bool Copy(char(&destination)[size], const char* source, size_t maxCount = _TRUNCATE)
		{
			return Copy(destination, size, source, maxCount);
		}

		inline bool Copy(wchar_t* destination, size_t size, const wchar_t* source, size_t maxCount = _TRUNCATE)
		{
			if (destination == nullptr || source == nullptr)
			{
				assert(false);
				return false;
			}

			if (maxCount != _TRUNCATE && size <= maxCount)
			{
				assert(false);
				maxCount = _TRUNCATE;
			}

			wcsncpy_s(destination, size, source, maxCount);

			return true;
		}

		template <size_t size>
		inline bool Copy(wchar_t(&destination)[size], const wchar_t* source, size_t maxCount = _TRUNCATE)
		{
			return Copy(destination, size, source, maxCount);
		}

		inline bool Concat(char* destination, size_t size, const char* source, size_t maxCount = _TRUNCATE)
		{
			if (destination == nullptr || source == nullptr)
			{
				assert(false);
				return false;
			}

			if (maxCount != _TRUNCATE && size <= maxCount)
			{
				assert(false);
				maxCount = _TRUNCATE;
			}

			strncat_s(destination, size, source, maxCount);

			return true;
		}

		template <size_t size>
		inline bool Concat(char(&destination)[size], const char* source, size_t maxCount = _TRUNCATE)
		{
			return Concat(destination, size, source, maxCount);
		}

		inline bool Concat(wchar_t* destination, size_t size, const wchar_t* source, size_t maxCount = _TRUNCATE)
		{
			if (destination == nullptr || source == nullptr)
			{
				assert(false);
				return false;
			}

			if (maxCount != _TRUNCATE && size <= maxCount)
			{
				assert(false);
				maxCount = _TRUNCATE;
			}

			wcsncat_s(destination, size, source, maxCount);

			return true;
		}

		template <size_t size>
		inline bool Concat(wchar_t(&destination)[size], const wchar_t* source, size_t maxCount = _TRUNCATE)
		{
			return Concat(destination, size, source, maxCount);
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

		std::vector<std::string> Tokenizer(const std::string& string, char delimiter);
		std::vector<std::wstring> Tokenizer(const std::wstring& string, wchar_t delimiter);

		char* ToUpper(char* string, size_t length);
		template <typename T, size_t size>
		char* ToUpper(char(&string)[size]) { return ToUpper(string, size); }

		wchar_t* ToUpper(wchar_t* string, size_t length);
		template <typename T, size_t size>
		wchar_t* ToUpper(wchar_t(&string)[size]) { return ToUpper(string, size); }

		void ToUpper(std::string& string);
		void ToUpper(std::wstring& string);

		char* ToLower(char* string, size_t length);
		template <typename T, size_t size>
		char* ToLower(char(&string)[size]) { return ToLower(string, size); }

		wchar_t* ToLower(wchar_t* string, size_t length);
		template <typename T, size_t size>
		wchar_t* ToLower(wchar_t(&string)[size]) { return ToLower(string, size); }

		void ToLower(std::string& string);
		void ToLower(std::wstring& string);

		template <typename T>
		std::string ToString(T value) { return std::to_string(value); }

		template <typename T>
		std::wstring ToStringW(T value) { return std::to_wstring(value); }

		template <typename T>
		T ToValue(const char* string);

		template <typename T>
		T ToValue(const wchar_t* string);

		std::string Format(const char* format, ...);
		std::wstring Format(const wchar_t* format, ...);
		std::string RandomString(uint32_t length, bool isOnlyAlphabet = true);
		std::wstring RandomStringW(uint32_t length, bool isOnlyAlphabet = true);

		std::string WideToMulti(const wchar_t* wideString, DWORD dwCodePage = CP_ACP);
		std::string WideToMulti(const std::wstring& wideString, DWORD dwCodePage = CP_ACP);
		std::wstring MultiToWide(const char* multiString, DWORD dwCodePage = CP_ACP);
		std::wstring MultiToWide(const std::string& multiString, DWORD dwCodePage = CP_ACP);

		std::string Utf8ToMulti(const wchar_t* wideString, DWORD dwCodePage = CP_UTF8);
		std::string Utf8ToMulti(const std::wstring& wideString, DWORD dwCodePage = CP_UTF8);
		std::wstring MultiToUtf8(const char* multiString, DWORD dwCodePage = CP_UTF8);
		std::wstring MultiToUtf8(const std::string& multiString, DWORD dwCodePage = CP_UTF8);
	};
}