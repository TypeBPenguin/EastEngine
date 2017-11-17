#include "stdafx.h"
#include "PipeData.h"

namespace EastEngine
{
	namespace PipeNetwork
	{
		CPipeData::CPipeData(int nDataType, uint32_t nCapacity)
			: m_nWritePos(0)
			, m_nReadPos(0)
			, m_nMaxSize(nCapacity)
		{
			m_header.nDataType = nDataType;
			m_header.nDataSize = nCapacity;
			m_vecBuffer.resize(nCapacity);
		}

		CPipeData::CPipeData(int nDataType, const char* pData, uint32_t nSize)
			: m_nWritePos(nSize - 1)
			, m_nReadPos(0)
			, m_nMaxSize(nSize)
		{
			m_header.nDataType = nDataType;
			m_header.nDataSize = nSize;
			m_vecBuffer.resize(nSize);

			CopyMemory(&m_vecBuffer[0], pData, nSize);
		}

		CPipeData::~CPipeData()
		{
		}

		template<typename T>
		void CPipeData::Write(T value)
		{
			return;
		}

		template <>
		void CPipeData::Write(int8_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(int8_t));
		}

		template <>
		void CPipeData::Write(int16_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(int16_t));
		}

		template <>
		void CPipeData::Write(int32_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(int32_t));
		}

		template <>
		void CPipeData::Write(int64_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(int64_t));
		}

		template <>
		void CPipeData::Write(uint8_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(uint8_t));
		}

		template <>
		void CPipeData::Write(uint16_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(uint16_t));
		}

		template <>
		void CPipeData::Write(uint32_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(uint32_t));
		}

		template <>
		void CPipeData::Write(uint64_t value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(uint64_t));
		}

		template <>
		void CPipeData::Write(float value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(float));
		}

		template <>
		void CPipeData::Write(double value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(double));
		}

		template <>
		void CPipeData::Write(bool value)
		{
			write(reinterpret_cast<char*>(&value), sizeof(bool));
		}

		template <>
		int8_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(int8_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			int8_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		int16_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(int16_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			int16_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		int32_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(int32_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			int32_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		int64_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(int64_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			int64_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		uint8_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(uint8_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			uint8_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		uint16_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(uint16_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			uint16_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		uint32_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(uint32_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			uint32_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		uint64_t CPipeData::Read()
		{
			uint32_t nSize = sizeof(uint64_t);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0;

			uint64_t ret = 0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		float CPipeData::Read()
		{
			uint32_t nSize = sizeof(float);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0.f;

			float ret = 0.f;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		double CPipeData::Read()
		{
			uint32_t nSize = sizeof(double);

			char* pData = read(nSize);
			if (pData == nullptr)
				return 0.0;

			double ret = 0.0;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		template <>
		bool CPipeData::Read()
		{
			uint32_t nSize = sizeof(bool);

			char* pData = read(nSize);
			if (pData == nullptr)
				return false;

			bool ret = false;
			CopyMemory(&ret, pData, nSize);

			m_nReadPos += nSize;

			return ret;
		}

		void CPipeData::WriteString(const char* str)
		{
			uint32_t nSize = strlen(str) + 1;
			write(str, nSize);
		}

		const char* CPipeData::ReadString()
		{
			char* str = nullptr;
			if (m_vecBuffer.empty())
				return str;

			if (m_nMaxSize <= m_nReadPos)
				return str;

			int nMove = 0;
			while (m_vecBuffer[m_nReadPos + nMove++] != '\0');

			str = &m_vecBuffer[m_nReadPos];

			m_nReadPos += nMove;

			return str;
		}

		uint32_t CPipeData::Packing()
		{
			m_nMaxSize = m_nWritePos;
			m_vecBuffer.resize(m_nMaxSize);

			m_header.nDataSize = m_nMaxSize;

			return m_nMaxSize;
		}

		void CPipeData::write(const char* pData, uint32_t nSize)
		{
			bool bNeedReSize = false;
			while (m_nMaxSize < m_nWritePos + nSize)
			{
				m_nMaxSize *= 2;
				bNeedReSize = true;
			}

			if (bNeedReSize)
			{
				m_vecBuffer.resize(m_nMaxSize);
			}

			char* pBuffer = &m_vecBuffer[m_nWritePos];
			CopyMemory(pBuffer, pData, nSize);

			m_nWritePos += nSize;
		}

		char* CPipeData::read(uint32_t nSize)
		{
			if (m_vecBuffer.empty())
				return nullptr;

			if (m_nMaxSize < m_nReadPos + nSize)
				return nullptr;

			return &m_vecBuffer[m_nReadPos];
		}
	}
}