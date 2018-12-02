#include "stdafx.h"
#include "FileStream.h"

#include "FileUtil.h"
#include "StringUtil.h"

namespace eastengine
{
	namespace file
	{
		static_assert(std::ios::in == eRead, "Openmode Mismatch");
		static_assert(std::ios::out == eWrite, "Openmode Mismatch");
		static_assert(std::ios::binary == eBinary, "Openmode Mismatch");

		class Stream::Impl
		{
		public:
			Impl() = default;
			~Impl() = default;

		public:
			std::fstream m_file;
			std::string m_path;
			OpenMode m_emOpenMode{ eNone };
			size_t m_fileSize{ 0 };

			std::vector<char> m_buffer;
		};

		Stream::Stream()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		Stream::~Stream()
		{
			Close();
		}

		bool Stream::Open(const char* fileName, OpenMode emMode)
		{
			m_pImpl->m_emOpenMode = emMode;

			if (m_pImpl->m_emOpenMode == OpenMode::eNone)
			{
				m_pImpl->m_file.open(fileName);
			}
			else
			{
				m_pImpl->m_file.open(fileName, m_pImpl->m_emOpenMode);
			}

			if (m_pImpl->m_file.fail())
				return false;

			if (m_pImpl->m_file.bad())
				return false;

			m_pImpl->m_fileSize = std::experimental::filesystem::file_size(fileName);

			m_pImpl->m_path = fileName;

			return true;
		}

		void Stream::Close()
		{
			m_pImpl->m_file.close();
		}

		bool Stream::Eof() const
		{
			return m_pImpl->m_file.eof();
		}

		void Stream::Clear()
		{
			m_pImpl->m_file.clear();
		}

		size_t Stream::GetFileSize() const
		{
			return m_pImpl->m_fileSize;
		}

		const std::string& Stream::GetFilePath() const
		{
			return m_pImpl->m_path;
		}

		uint32_t Stream::GetOpenMode() const
		{
			return m_pImpl->m_emOpenMode;
		}

		const BYTE* Stream::GetBuffer()
		{
			m_pImpl->m_buffer.resize(m_pImpl->m_fileSize);
			Read(m_pImpl->m_buffer.data(), m_pImpl->m_fileSize);

			return reinterpret_cast<const BYTE*>(m_pImpl->m_buffer.data());
		}

		Stream& Stream::operator << (int8_t value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(int8_t));

			return *this;
		}

		Stream& Stream::operator >> (int8_t& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(int8_t));

			return *this;
		}

		Stream& Stream::operator << (int16_t value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(int16_t));

			return *this;
		}

		Stream& Stream::operator >> (int16_t& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(int16_t));

			return *this;
		}

		Stream& Stream::operator << (int value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(int));

			return *this;
		}

		Stream& Stream::operator >> (int& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(int));

			return *this;
		}

		Stream& Stream::operator << (DWORD value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(DWORD));

			return *this;
		}

		Stream& Stream::operator >> (DWORD& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(DWORD));

			return *this;
		}

		Stream& Stream::operator << (uint8_t value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(uint8_t));

			return *this;
		}

		Stream& Stream::operator >> (uint8_t& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(uint8_t));

			return *this;
		}

		Stream& Stream::operator << (uint16_t value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(uint16_t));

			return *this;
		}

		Stream& Stream::operator >> (uint16_t& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(uint16_t));

			return *this;
		}

		Stream& Stream::operator << (uint32_t value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(uint32_t));

			return *this;
		}

		Stream& Stream::operator >> (uint32_t& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));

			return *this;
		}

		Stream& Stream::operator << (float value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(float));

			return *this;
		}

		Stream& Stream::operator >> (float& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(float));

			return *this;
		}

		Stream& Stream::operator << (double value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(double));

			return *this;
		}

		Stream& Stream::operator >> (double& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(double));

			return *this;
		}

		Stream& Stream::operator << (bool value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file << value;
			else
				m_pImpl->m_file.write(reinterpret_cast<char*>(&value), sizeof(bool));

			return *this;
		}

		Stream& Stream::operator >> (bool& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
				m_pImpl->m_file >> value;
			else
				m_pImpl->m_file.read(reinterpret_cast<char*>(&value), sizeof(bool));

			return *this;
		}

		Stream& Stream::operator << (const std::string& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
			{
				m_pImpl->m_file << value;
			}
			else
			{
				// + 1 해주는거 지워야함
				size_t size = value.length() + 1;
				*this << static_cast<uint32_t>(size);
				m_pImpl->m_file.write(value.c_str(), size);
			}

			return *this;
		}

		Stream& Stream::operator >> (std::string& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
			{
				m_pImpl->m_file >> value;
			}
			else
			{
				uint32_t size = 0;

				*this >> size;

				if (size <= 0)
					return *this;

				// + 1 해주고 있는거 지워야함
				value.resize(size);
				m_pImpl->m_file.read(value.data(), size);

				value = value.substr(0, value.size() - 1);
			}

			return *this;
		}

		Stream& Stream::operator << (const std::wstring& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
			{
				m_pImpl->m_file << string::WideToMulti(value.c_str(), CP_ACP);
			}
			else
			{
				size_t size = value.length() + 1;
				*this << static_cast<uint32_t>(size);
				m_pImpl->m_file.write(reinterpret_cast<const char*>(value.c_str()), size * 2);
			}

			return *this;
		}
		Stream& Stream::operator >> (std::wstring& value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
			{
				std::string str;
				m_pImpl->m_file >> str;

				value = string::MultiToWide(str.c_str(), CP_ACP);
			}
			else
			{
				uint32_t size = 0;

				*this >> size;

				if (size <= 0)
					return *this;

				std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
				m_pImpl->m_file.read(reinterpret_cast<char*>(buf.get()), size * 2);
				value = buf.get();
			}

			return *this;
		}

		Stream& Stream::operator << (const char* value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
			{
				m_pImpl->m_file << value;
			}
			else
			{
				if (value == nullptr)
				{
					return *this << "";
				}

				uint32_t size = (uint32_t)strlen(value) + 1;
				*this << size;
				m_pImpl->m_file.write(value, size);
			}

			return *this;
		}

		Stream& Stream::operator << (wchar_t* value)
		{
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
			{
				m_pImpl->m_file << value;

				//m_pImpl->m_file << AloStr::WideToMulti(value, CP_ACP);
			}
			else
			{
				if (value == nullptr)
				{
					return *this << L"";
				}

				uint32_t size = (uint32_t)wcslen(value) + 1;
				*this << size;
				m_pImpl->m_file.write(reinterpret_cast<char*>(value), size * 2);
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
			if ((m_pImpl->m_emOpenMode & OpenMode::eBinary) == 0)
			{
				m_pImpl->m_file << pValue;
			}
			else
			{
				if (pValue == nullptr)
				{
					return *this << "";
				}

				*this << nLength;
				m_pImpl->m_file.write(pValue, nLength);
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

		Stream& Stream::Read(char* pBuffer, size_t length)
		{
			m_pImpl->m_file.read(pBuffer, length);
			return *this;
		}

		void Stream::ReadLine(std::string& str)
		{
			std::getline(m_pImpl->m_file, str);
		}
	}
}