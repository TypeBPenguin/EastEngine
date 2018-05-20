#pragma once

#include <fstream>

namespace eastengine
{
	namespace file
	{
		enum OpenMode
		{
			eNone = 0,
			eRead = 0x01,
			eWrite = 0x02,
			eBinary = 0x20,
		};

		class Stream
		{
		public:
			Stream();
			~Stream();

		public:
			bool Open(const char* fileName, uint32_t emMode = OpenMode::eNone);
			void Close() { m_file.close(); }

			bool Eof() { return m_file.eof(); }

			void Clear() { m_file.clear(); }
			void Seekg(std::streampos pos, uint32_t state) { m_file.seekp(pos, state); }
			std::streampos Tellg() { m_file.tellg(); }

			size_t GetFileSize() const { return m_nFileSize; }

		public:
			Stream& operator << (int8_t value);
			Stream& operator >> (int8_t& value);
			Stream& operator << (int16_t value);
			Stream& operator >> (int16_t& value);
			Stream& operator << (int value);
			Stream& operator >> (int& value);
			Stream& operator << (DWORD value);
			Stream& operator >> (DWORD& value);
			Stream& operator << (uint8_t value);
			Stream& operator >> (uint8_t& value);
			Stream& operator << (uint16_t value);
			Stream& operator >> (uint16_t& value);
			Stream& operator << (uint32_t value);
			Stream& operator >> (uint32_t& value);
			Stream& operator << (float value);
			Stream& operator >> (float& value);
			Stream& operator << (double value);
			Stream& operator >> (double& value);
			Stream& operator << (bool value);
			Stream& operator >> (bool& value);
			Stream& operator << (const std::string& value);
			Stream& operator >> (std::string& value);
			Stream& operator << (const std::wstring& value);
			Stream& operator >> (std::wstring& value);
			Stream& operator << (const char* value);
			Stream& operator << (wchar_t* value);

		public:
			Stream& Write(const float* pValue, uint32_t nCount = 1);
			Stream& Write(const double* pValue, uint32_t nCount = 1);
			Stream& Write(const int8_t* pValue, uint32_t nCount = 1);
			Stream& Write(const int16_t* pValue, uint32_t nCount = 1);
			Stream& Write(const int32_t* pValue, uint32_t nCount = 1);
			Stream& Write(const int64_t* pValue, uint32_t nCount = 1);
			Stream& Write(const uint8_t* pValue, uint32_t nCount = 1);
			Stream& Write(const uint16_t* pValue, uint32_t nCount = 1);
			Stream& Write(const uint32_t* pValue, uint32_t nCount = 1);
			Stream& Write(const uint64_t* pValue, uint32_t nCount = 1);

			Stream& Write(const char* pValue, uint32_t nLength = 1);

			Stream& Read(float* pBuffer, uint32_t nCount = 1);
			Stream& Read(double* pBuffer, uint32_t nCount = 1);
			Stream& Read(int8_t* pBuffer, uint32_t nCount = 1);
			Stream& Read(int16_t* pBuffer, uint32_t nCount = 1);
			Stream& Read(int32_t* pBuffer, uint32_t nCount = 1);
			Stream& Read(int64_t* pBuffer, uint32_t nCount = 1);
			Stream& Read(uint8_t* pBuffer, uint32_t nCount = 1);
			Stream& Read(uint16_t* pBuffer, uint32_t nCount = 1);
			Stream& Read(uint32_t* pBuffer, uint32_t nCount = 1);
			Stream& Read(uint64_t* pBuffer, uint32_t nCount = 1);

			Stream& Read(char* pBuffer, uint32_t nLength);

			void ReadLine(std::string& str);

		public:
			const std::string& GetFilePath() const { return m_strPath; }
			uint32_t GetOpenMode() const { return m_nOpenMode; }

		private:
			std::fstream m_file;
			std::string m_strPath;
			uint32_t m_nOpenMode;
			size_t m_nFileSize;
		};
	}
}