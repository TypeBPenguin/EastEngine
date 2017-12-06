#include "stdafx.h"
#include "FileStream.h"

#include "FileUtil.h"
#include "StringUtil.h"

namespace EastEngine
{
	namespace File
	{
		FileStream::FileStream()
			: m_nFlag(EmState::eNone)
			, m_nDataSize(0)
		{
		}

		FileStream::~FileStream()
		{
			Close();
		}

		FileStream& FileStream::operator << (int value)
		{
			if (m_nFlag == EmState::eNone)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(int));

			return *this;
		}

		FileStream& FileStream::operator >> (int& value)
		{
			if (m_nFlag == EmState::eNone)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(int));

			return *this;
		}

		FileStream& FileStream::operator << (DWORD value)
		{
			if (m_nFlag == EmState::eNone)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(DWORD));

			return *this;
		}

		FileStream& FileStream::operator >> (DWORD& value)
		{
			if (m_nFlag == EmState::eNone)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(DWORD));

			return *this;
		}

		FileStream& FileStream::operator << (uint32_t value)
		{
			if (m_nFlag == EmState::eNone)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(uint32_t));

			return *this;
		}

		FileStream& FileStream::operator >> (uint32_t& value)
		{
			if (m_nFlag == EmState::eNone)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));

			return *this;
		}

		FileStream& FileStream::operator << (float value)
		{
			if (m_nFlag == EmState::eNone)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(float));

			return *this;
		}

		FileStream& FileStream::operator >> (float& value)
		{
			if (m_nFlag == EmState::eNone)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(float));

			return *this;
		}

		FileStream& FileStream::operator << (double value)
		{
			if (m_nFlag == EmState::eNone)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(double));

			return *this;
		}

		FileStream& FileStream::operator >> (double& value)
		{
			if (m_nFlag == EmState::eNone)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(double));

			return *this;
		}

		FileStream& FileStream::operator << (bool value)
		{
			if (m_nFlag == EmState::eNone)
				m_file << value;
			else
				m_file.write(reinterpret_cast<char*>(&value), sizeof(bool));

			return *this;
		}

		FileStream& FileStream::operator >> (bool& value)
		{
			if (m_nFlag == EmState::eNone)
				m_file >> value;
			else
				m_file.read(reinterpret_cast<char*>(&value), sizeof(bool));

			return *this;
		}

		FileStream& FileStream::operator << (const std::string& value)
		{
			if (m_nFlag == EmState::eNone)
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

		FileStream& FileStream::operator >> (std::string& value)
		{
			if (m_nFlag == EmState::eNone)
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

		FileStream& FileStream::operator << (const std::wstring& value)
		{
			if (m_nFlag == EmState::eNone)
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
		FileStream& FileStream::operator >> (std::wstring& value)
		{
			if (m_nFlag == EmState::eNone)
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

		FileStream& FileStream::operator << (const char* value)
		{
			if (m_nFlag == EmState::eNone)
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

		FileStream& FileStream::operator << (wchar_t* value)
		{
			if (m_nFlag == EmState::eNone)
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

		FileStream& FileStream::Write(const float* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		FileStream& FileStream::Write(const double* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		FileStream& FileStream::Write(const int* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		FileStream& FileStream::Write(const uint32_t* pValue, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this << pValue[i];
			}

			return *this;
		}

		FileStream& FileStream::Write(const char* pValue, uint32_t nLength)
		{
			if (m_nFlag == EmState::eNone)
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

		FileStream& FileStream::Read(float* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		FileStream& FileStream::Read(double* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		FileStream& FileStream::Read(int* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		FileStream& FileStream::Read(uint32_t* pBuffer, uint32_t nCount)
		{
			for (uint32_t i = 0; i < nCount; ++i)
			{
				*this >> pBuffer[i];
			}

			return *this;
		}

		FileStream& FileStream::Read(char* pBuffer, uint32_t nLength)
		{
			m_file.read(pBuffer, nLength);

			return *this;
		}

		bool FileStream::Open(const char* fileName, uint32_t state/*EM_FILE_STATE*/, uint32_t* pDataSize)
		{
			m_nFlag = state;

			if (m_nFlag == EmState::eNone)
			{
				m_file.open(fileName);
			}
			else
			{
				if ((state & EmState::eRead) != 0)
				{
					m_nFlag |= std::ios::in;
				}

				if ((state & EmState::eWrite) != 0)
				{
					m_nFlag |= std::ios::out;
				}

				if ((state & EmState::eBinary) != 0)
				{
					m_nFlag |= std::ios::binary;
				}

				m_file.open(fileName, m_nFlag);
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