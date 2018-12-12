#include "stdafx.h"
#include "StringUtil.h"

#include "Math.h"

#include <boost/tokenizer.hpp>
#include <atlstr.h>

namespace eastengine
{
	namespace string
	{
		std::vector<std::string> Tokenizer(const char* string, const char* strDelimiter)
		{
			if (string == nullptr)
				return std::vector<std::string>();

			std::string str = string;
			return Tokenizer(str, strDelimiter);
		}

		std::vector<std::string> Tokenizer(const std::string& string, const char* strDelimiter)
		{
			if (string.empty() == true)
				return std::vector<std::string>();

			boost::char_separator<char> sep(strDelimiter);
			boost::tokenizer<boost::char_separator<char>> tokens(string, sep);

			return std::vector<std::string>(tokens.begin(), tokens.end());
		}

		char* ToUpper(char* string, uint32_t nLength)
		{
			if (string == nullptr)
				return nullptr;

			_strupr_s(string, nLength);

			return string;
		}

		char* ToLower(char* string, uint32_t nLength)
		{
			if (string == nullptr)
				return nullptr;

			_strlwr_s(string, nLength);

			return string;
		}

		//template <> std::string ToString(int8_t value) { return std::to_string(value); }
		//template <> std::string ToString(int16_t value) { return std::to_string(value); }
		//template <> std::string ToString(int32_t value) { return std::to_string(value); }
		//template <> std::string ToString(int64_t value) { return std::to_string(value); }
		//template <> std::string ToString(uint8_t value) { return std::to_string(value); }
		//template <> std::string ToString(uint16_t value) { return std::to_string(value); }
		//template <> std::string ToString(uint32_t value) { return std::to_string(value); }
		//template <> std::string ToString(uint64_t value) { return std::to_string(value); }
		//template <> std::string ToString(float value) { return std::to_string(value); }
		//template <> std::string ToString(double value) { return std::to_string(value); }
		//template <> std::string ToString(long double value) { return std::to_string(value); }

		template <> int32_t ToValue(const char* string) { if (string == nullptr) return 0; return std::stoi(string); }
		template <> int64_t ToValue(const char* string) { if (string == nullptr) return 0; return std::stoll(string); }
		template <> float ToValue(const char* string) { if (string == nullptr) return 0.f; return std::stof(string); }
		template <> double ToValue(const char* string) { if (string == nullptr) return 0.0; return std::stod(string); }
		template <> long double ToValue(const char* string) { if (string == nullptr) return 0.0l; return std::stold(string); }

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

		std::string RandomString(uint32_t nLength, bool isOnlyAlphabet)
		{
			std::string strRandom(nLength, 0);

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

				std::generate_n(strRandom.begin(), nLength, CharRandomOnlyAlphabet);
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

				std::generate_n(strRandom.begin(), nLength, CharRandom);
			}

			return strRandom;
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