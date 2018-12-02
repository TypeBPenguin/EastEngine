#pragma once

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

			eReadBinary = eRead | eBinary,
			eWriteBinary = eWrite | eBinary,

			eReadWrite = eRead | eWrite,
			eReadWriteBinary = eRead | eWrite | eBinary,
		};

		class Stream
		{
		public:
			Stream();
			~Stream();

		public:
			bool Open(const char* fileName, OpenMode emMode = OpenMode::eNone);
			void Close();

			bool Eof() const;

			void Clear();

		public:
			size_t GetFileSize() const;
			const std::string& GetFilePath() const;
			uint32_t GetOpenMode() const;

		public:
			const BYTE* GetBuffer();

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

			Stream& Read(char* pBuffer, size_t length);

			void ReadLine(std::string& str);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;

		public:
			template <typename T>
			static const T* To(const BYTE** ppBuffer, size_t count = 1)
			{
				assert(count > 0);

				const T* pPointer = reinterpret_cast<const T*>(*ppBuffer);
				*ppBuffer += sizeof(T) * count;

				return pPointer;
			}

			static const char* ToString(const BYTE** ppBuffer)
			{
				const uint32_t* pLength = reinterpret_cast<const uint32_t*>(*ppBuffer);
				*ppBuffer += sizeof(uint32_t);

				if (*pLength == 0)
					return "";

				const char* pName = reinterpret_cast<const char*>(*ppBuffer);
				*ppBuffer += *pLength;

				return pName;
			}
		};
	}
}