#include "stdafx.h"
#include "StringUtil.h"

#include "Math.h"

#include <algorithm>

namespace est
{
	namespace string
	{
		std::vector<std::string> Tokenizer(const std::string& string, char delimiter)
		{
			if (string.empty() == true)
				return std::vector<std::string>();

			std::vector<std::string> tokens;

			std::istringstream iss(string);
			std::string token;
			while (getline(iss, token, delimiter))
			{
				tokens.emplace_back(token);
			}
			return tokens;
		}

		std::vector<std::wstring> Tokenizer(const std::wstring& string, wchar_t delimiter)
		{
			if (string.empty() == true)
				return std::vector<std::wstring>();

			std::vector<std::wstring> tokens;

			std::wistringstream iss(string);
			std::wstring token;
			while (getline(iss, token, delimiter))
			{
				tokens.emplace_back(token);
			}
			return tokens;
		}

		char* ToUpper(char* string, size_t length)
		{
			if (string == nullptr)
				return nullptr;

			_strupr_s(string, length);

			return string;
		}

		wchar_t* ToUpper(wchar_t* string, size_t length)
		{
			if (string == nullptr)
				return nullptr;

			_wcsupr_s(string, length);

			return string;
		}

#pragma warning(push)
#pragma warning(disable:4244)
		void ToUpper(std::string& string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::toupper);
		}

		void ToUpper(std::wstring& string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::toupper);
		}
#pragma warning(pop)

		char* ToLower(char* string, size_t length)
		{
			if (string == nullptr)
				return nullptr;

			_strlwr_s(string, length);

			return string;
		}

		wchar_t* ToLower(wchar_t* string, size_t length)
		{
			if (string == nullptr)
				return nullptr;

			_wcslwr_s(string, length);

			return string;
		}

#pragma warning(push)
#pragma warning(disable:4244)
		void ToLower(std::string& string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::tolower);
		}

		void ToLower(std::wstring& string)
		{
			std::transform(string.begin(), string.end(), string.begin(), ::tolower);
		}
#pragma warning(pop)

		template <> int32_t ToValue(const char* string) { if (string == nullptr) return 0; return std::stoi(string); }
		template <> int64_t ToValue(const char* string) { if (string == nullptr) return 0; return std::stoll(string); }
		template <> float ToValue(const char* string) { if (string == nullptr) return 0.f; return std::stof(string); }
		template <> double ToValue(const char* string) { if (string == nullptr) return 0.0; return std::stod(string); }
		template <> long double ToValue(const char* string) { if (string == nullptr) return 0.0l; return std::stold(string); }

		template <> int32_t ToValue(const wchar_t* string) { if (string == nullptr) return 0; return std::stoi(string); }
		template <> int64_t ToValue(const wchar_t* string) { if (string == nullptr) return 0; return std::stoll(string); }
		template <> float ToValue(const wchar_t* string) { if (string == nullptr) return 0.f; return std::stof(string); }
		template <> double ToValue(const wchar_t* string) { if (string == nullptr) return 0.0; return std::stod(string); }
		template <> long double ToValue(const wchar_t* string) { if (string == nullptr) return 0.0l; return std::stold(string); }

		std::string Format(const char* format, ...)
		{
			va_list args;
			va_start(args, format);
			uint32_t size = std::vsnprintf(nullptr, 0, format, args) + 1;
			va_end(args);

			std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
			va_start(args, format);
			std::vsnprintf(buf.get(), size, format, args);
			va_end(args);

			return { buf.get() };
		}

		std::wstring Format(const wchar_t* format, ...)
		{
			va_list args;
			va_start(args, format);
			uint32_t size = std::vswprintf(nullptr, 0, format, args) + 1;
			va_end(args);

			std::unique_ptr<wchar_t[]> buf = std::make_unique<wchar_t[]>(size);
			va_start(args, format);
			std::vswprintf(buf.get(), size, format, args);
			va_end(args);

			return { buf.get() };
		}

		std::string RandomString(uint32_t length, bool isOnlyAlphabet)
		{
			std::string randomString(length, 0);

			if (isOnlyAlphabet == true)
			{
				auto CharRandomOnlyAlphabet = []() -> char
				{
					static const char CharacterSet[] =
					{
						'A','B','C','D','E','F',
						'G','H','I','J','K',
						'L','M','N','O','P',
						'Q','R','S','T','U',
						'V','W','X','Y','Z',
						'a','b','c','d','e','f',
						'g','h','i','j','k',
						'l','m','n','o','p',
						'q','r','s','t','u',
						'v','w','x','y','z'
					};

					return CharacterSet[math::Random<int>(0, sizeof(CharacterSet) - 1)];
				};

				std::generate_n(randomString.begin(), length, CharRandomOnlyAlphabet);
			}
			else
			{
				auto CharRandom = []() -> char
				{
					static const char CharacterSet[] =
					{
						'0','1','2','3','4',
						'5','6','7','8','9',
						'A','B','C','D','E','F',
						'G','H','I','J','K',
						'L','M','N','O','P',
						'Q','R','S','T','U',
						'V','W','X','Y','Z',
						'a','b','c','d','e','f',
						'g','h','i','j','k',
						'l','m','n','o','p',
						'q','r','s','t','u',
						'v','w','x','y','z'
					};

					return CharacterSet[math::Random<int>(0, sizeof(CharacterSet) - 1)];
				};

				std::generate_n(randomString.begin(), length, CharRandom);
			}

			return randomString;
		}

		std::wstring RandomStringW(uint32_t length, bool isOnlyAlphabet)
		{
			std::wstring randomString(length, 0);

			if (isOnlyAlphabet == true)
			{
				auto CharRandomOnlyAlphabet = []() -> wchar_t
				{
					static const wchar_t CharacterSet[] =
					{
						'A','B','C','D','E','F',
						'G','H','I','J','K',
						'L','M','N','O','P',
						'Q','R','S','T','U',
						'V','W','X','Y','Z',
						'a','b','c','d','e','f',
						'g','h','i','j','k',
						'l','m','n','o','p',
						'q','r','s','t','u',
						'v','w','x','y','z'
					};

					return CharacterSet[math::Random<int>(0, sizeof(CharacterSet) - 1)];
				};

				std::generate_n(randomString.begin(), length, CharRandomOnlyAlphabet);
			}
			else
			{
				auto CharRandom = []() -> wchar_t
				{
					static const wchar_t CharacterSet[] =
					{
						'0','1','2','3','4',
						'5','6','7','8','9',
						'A','B','C','D','E','F',
						'G','H','I','J','K',
						'L','M','N','O','P',
						'Q','R','S','T','U',
						'V','W','X','Y','Z',
						'a','b','c','d','e','f',
						'g','h','i','j','k',
						'l','m','n','o','p',
						'q','r','s','t','u',
						'v','w','x','y','z'
					};

					return CharacterSet[math::Random<int>(0, sizeof(CharacterSet) - 1)];
				};

				std::generate_n(randomString.begin(), length, CharRandom);
			}

			return randomString;
		}

		std::string WideToMulti(const wchar_t* strWide, DWORD dwCodePage)
		{
			return { CW2A(strWide, dwCodePage).m_psz };
		}

		std::string WideToMulti(const std::wstring& strWide, DWORD dwCodePage)
		{
			return WideToMulti(strWide.c_str(), dwCodePage);
		}

		std::wstring MultiToWide(const char* strMulti, DWORD dwCodePage)
		{
			return { CA2W(strMulti, dwCodePage).m_psz };
		}

		std::wstring MultiToWide(const std::string& strMulti, DWORD dwCodePage)
		{
			return MultiToWide(strMulti.c_str(), dwCodePage);
		}

		std::string Utf8ToMulti(const wchar_t* strWide, DWORD dwCodePage)
		{
			return { CW2A(strWide, dwCodePage).m_psz };
		}

		std::string Utf8ToMulti(const std::wstring& strWide, DWORD dwCodePage)
		{
			return Utf8ToMulti(strWide.c_str(), dwCodePage);
		}

		std::wstring MultiToUtf8(const char* strMulti, DWORD dwCodePage)
		{
			return { CA2W(strMulti, dwCodePage).m_psz };
		}

		std::wstring MultiToUtf8(const std::string& strMulti, DWORD dwCodePage)
		{
			return MultiToUtf8(strMulti.c_str(), dwCodePage);
		}
	};
}