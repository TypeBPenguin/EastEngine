#include "stdafx.h"
#include "FileStream.h"

#include "FileUtil.h"
#include "StringUtil.h"

namespace EastEngine
{
	namespace File
	{
		static_assert(std::ios::in == eRead, "Openmode Mismatch");
		static_assert(std::ios::out == eWrite, "Openmode Mismatch");
		static_assert(std::ios::binary == eBinary, "Openmode Mismatch");

		Stream::Stream()
			: m_nOpenMode(OpenMode::eNone)
			, m_nDataSize(0)
		{
		}

		Stream::~Stream()
		{
			Close();
		}

		Stream& Stream::operator << (int8_t value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(int8_t));

			return *this;
		}

		Stream& Stream::operator >> (int8_t& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(int8_t));

			return *this;
		}

		Stream& Stream::operator << (int16_t value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(int16_t));

			return *this;
		}

		Stream& Stream::operator >> (int16_t& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(int16_t));

			return *this;
		}

		Stream& Stream::operator << (int value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(int));

			return *this;
		}

		Stream& Stream::operator >> (int& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(int));

			return *this;
		}

		Stream& Stream::operator << (DWORD value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(DWORD));

			return *this;
		}

		Stream& Stream::operator >> (DWORD& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(DWORD));

			return *this;
		}

		Stream& Stream::operator << (uint8_t value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(uint8_t));

			return *this;
		}

		Stream& Stream::operator >> (uint8_t& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(uint8_t));

			return *this;
		}

		Stream& Stream::operator << (uint16_t value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(uint16_t));

			return *this;
		}

		Stream& Stream::operator >> (uint16_t& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(uint16_t));

			return *this;
		}

		Stream& Stream::operator << (uint32_t value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(uint32_t));

			return *this;
		}

		Stream& Stream::operator >> (uint32_t& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));

			return *this;
		}

		Stream& Stream::operator << (float value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(float));

			return *this;
		}

		Stream& Stream::operator >> (float& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(float));

			return *this;
		}

		Stream& Stream::operator << (double value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(double));

			return *this;
		}

		Stream& Stream::operator >> (double& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(double));

			return *this;
		}

		Stream& Stream::operator << (bool value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(bool));

			return *this;
		}

		Stream& Stream::operator >> (bool& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(bool));

			return *this;
		}

		Stream& Stream::operator << (const std::string& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
			{
				m_file << value;
			}
			else
			{
				uint32_t size = value.length() + 1;
				*this << size;
				m_file.write(value.c_str(), size);
			}

			return *this;
		}

		Stream& Stream::operator >> (std::string& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
			{
				m_file >> value;
			}
			else
			{
				uint32_t size = 0;

				*this >> size;

				if (size <= 0)
					return *this;

				std::unique_ptr<char[]> buf(new char[size]);
				m_file.read(buf.get(), size);
				value = buf.get();
			}

			return *this;
		}

		Stream& Stream::operator << (const std::wstring& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
			{
				m_file << String::WideToMulti(value.c_str(), CP_ACP);
			}
			else
			{
				uint32_t size = value.length() + 1;
				*this << size;
				m_file.write(reinterpret_cast<const char*>(value.c_str()), size * 2);
			}

			return *this;
		}
		Stream& Stream::operator >> (std::wstring& value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
			{
				std::string str;
				m_file >> str;

				value = String::MultiToWide(str.c_str(), CP_ACP);
			}
			else
			{
				uint32_t size = 0;

				*this >> size;

				if (size <= 0)
					return *this;

				std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
				m_file.read(reinterpret_cast<char*>(buf.get()), size * 2);
				value = buf.get();
			}

			return *this;
		}

		Stream& Stream::operator << (const char* value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
			{
				m_file << value;
			}
			else
			{
				if (value == nullptr)
				{
					return *this << "";
				}

				uint32_t size = (uint32_t)strlen(value) + 1;
				*this << size;
				m_file.write(value, size);
			}

			return *this;
		}

		Stream& Stream::operator << (wchar_t* value)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
			{
				m_file << value;

				//m_file << AloStr::WideToMulti(value, CP_ACP);
			}
			else
			{
				if (value == nullptr)
				{
					return *this << L"";
				}

				uint32_t size = (uint32_t)wcslen(value) + 1;
				*this << size;
				m_file.write(reinterpret_cast<char*>(value), size * 2);
			}

			return *this;
		}

		Stream& Stream::Write(const float* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const double* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const int8_t* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const int16_t* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const int32_t* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const uint8_t* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const uint16_t* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const uint32_t* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		Stream& Stream::Write(const char* pValue, uint32_t nLength)
		{
			if ((m_nOpenMode & OpenMode::eBinary) == 0)
			{
				m_file << pValue;
			}
			else
			{
				if (pValue == nullptr)
				{
					return *this << "";
				}

				*this << nLength;
				m_file.write(pValue, nLength);
			}

			return *this;
		}

		Stream& Stream::Read(float* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(double* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(int8_t* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(int16_t* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(int32_t* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(uint8_t* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(uint16_t* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(uint32_t* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		Stream& Stream::Read(char* pBuffer, uint32_t nLength)
		{
			m_file.read(pBuffer, nLength);

			return *this;
		}

		bool Stream::Open(const char* fileName, uint32_t emMode, uint32_t* pDataSize)
		{
			m_nOpenMode = emMode;

			if (m_nOpenMode == OpenMode::eNone)
			{
				m_file.open(fileName);
			}
			else
			{
				if ((emMode & OpenMode::eRead) != 0)
				{
					m_nOpenMode |= std::ios::in;
				}

				if ((emMode & OpenMode::eWrite) != 0)
				{
					m_nOpenMode |= std::ios::out;
				}

				if ((emMode & OpenMode::eBinary) != 0)
				{
					m_nOpenMode |= std::ios::binary;
				}

				m_file.open(fileName, m_nOpenMode);
			}

			if (m_file.fail())
				return false;

			if (m_file.bad())
				return false;

			m_file.seekg(0, std::ios::end);
			m_nDataSize = static_cast<uint32_t>(m_file.tellg());
			m_file.seekg(0, std::ios::beg);

			if (pDataSize != nullptr)
			{
				*pDataSize = m_nDataSize;
			}

			m_strPath = File::GetFilePath(fileName);

			return true;
		}
	}
}