#pragma once

#include <fstream>

namespace EastEngine
{
	namespace File
	{
		namespace EmState
		{
			enum Type
			{
				eNone = 0,
				eRead = 1,
				eWrite = 1 << 1,
				eBinary = 1 << 2,
			};
		}

		class FileStream
		{
		public:
			FileStream();
			~FileStream();

		public:
			bool Open(const char* fileName, uint32_t nState = EmState::eNone/*EM_FILE_STATE*/, uint32_t* pDataSize = nullptr);
			void Close() { m_file.close(); }

			bool Eof() { return m_file.eof(); }

			void Clear() { m_file.clear(); }
			void Seekg(std::streampos pos, uint32_t state) { m_file.seekg(pos, state); }
			std::streampos Tellg() { m_file.tellg(); }

			void GetLine(std::string& str) { std::getline(m_file, str); }

			uint32_t GetDataSize() { return m_nDataSize; }

		public:
			FileStream& operator << (int value);
			FileStream& operator >> (int& value);
			FileStream& operator << (DWORD value);
			FileStream& operator >> (DWORD& value);
			FileStream& operator << (uint32_t value);
			FileStream& operator >> (uint32_t& value);
			FileStream& operator << (float value);
			FileStream& operator >> (float& value);
			FileStream& operator << (double value);
			FileStream& operator >> (double& value);
			FileStream& operator << (bool value);
			FileStream& operator >> (bool& value);
			FileStream& operator << (const std::string& value);
			FileStream& operator >> (std::string& value);
			FileStream& operator << (const std::wstring& value);
			FileStream& operator >> (std::wstring& value);
			FileStream& operator << (const char* value);
			FileStream& operator << (wchar_t* value);

		public:
			FileStream& FileStream::Write(const float* pValue, uint32_t nCount);
			FileStream& FileStream::Write(const double* pValue, uint32_t nCount);
			FileStream& FileStream::Write(const int* pValue, uint32_t nCount);
			FileStream& FileStream::Write(const uint32_t* pValue, uint32_t nCount);
			FileStream& FileStream::Write(const uint64_t* pValue, uint32_t nCount);

			FileStream& FileStream::Write(const char* pValue, uint32_t nLength);

			FileStream& FileStream::Read(float* pBuffer, uint32_t nCount);
			FileStream& FileStream::Read(double* pBuffer, uint32_t nCount);
			FileStream& FileStream::Read(int* pBuffer, uint32_t nCount);
			FileStream& FileStream::Read(uint32_t* pBuffer, uint32_t nCount);
			FileStream& FileStream::Read(uint64_t* pBuffer, uint32_t nCount);

			FileStream& FileStream::Read(char* pBuffer, uint32_t nLength);

		public:
			const std::string& GetPath() { return m_strPath; }

		private:
			std::fstream m_file;
			std::string m_strPath;
			uint32_t m_nFlag;
			uint32_t m_nDataSize;
		};
	}
}