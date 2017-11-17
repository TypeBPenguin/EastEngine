#pragma once

namespace EastEngine
{
	namespace PipeNetwork
	{
		struct PipeDataHeader
		{
			int nDataType = 0;
			int nDataSize = 0;
		};

		class CPipeData
		{
		public:
			CPipeData(int nDataType = -1, uint32_t nCapacity = 256);
			CPipeData(int nDataType, const char* pData, uint32_t nSize);
			~CPipeData();

			template <typename T>
			void Write(T value);
			template <typename T>
			T Read();

			void WriteString(const char* str);
			const char* ReadString();

			uint32_t Packing();

			const PipeDataHeader& GetHeader() const { return m_header; }

			const char* GetData() const { return &m_vecBuffer.front(); }
			char* GetBuffer() { return &m_vecBuffer[0]; }

			uint32_t GetDataSize() const { return m_nWritePos; }
			uint32_t GetBufferSize() const { return m_nMaxSize; }

		private:
			void write(const char* pData, uint32_t nSize);
			char* read(uint32_t nSize);

		private:
			PipeDataHeader m_header;
			std::vector<char> m_vecBuffer;
			uint32_t m_nWritePos;
			uint32_t m_nReadPos;
			uint32_t m_nMaxSize;
		};
	}
}