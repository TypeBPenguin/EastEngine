#pragma once

#include <fstream>

#include "Binary.h"

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
			BinaryReader GetBinaryReader();

		public:
			template <typename T>
			Stream& operator << (const T& value)
			{
				if ((m_emOpenMode & OpenMode::eBinary) == 0)
				{
					m_file << value;
				}
				else
				{
					m_file.write(reinterpret_cast<const char*>(&value), sizeof(T));
				}
				return *this;
			}

			Stream& operator << (const char* value)
			{
				if ((m_emOpenMode & OpenMode::eBinary) == 0)
				{
					m_file << value;
				}
				else
				{
					const size_t size = strlen(value) + 1;
					*this << static_cast<uint32_t>(size);
					m_file.write(value, size);
				}
				return *this;
			}

			Stream& operator << (const std::string& value)
			{
				if ((m_emOpenMode & OpenMode::eBinary) == 0)
				{
					m_file << value.c_str();
				}
				else
				{
					const size_t size = value.length() + 1;
					*this << static_cast<uint32_t>(size);
					m_file.write(value.c_str(), size);
				}
				return *this;
			}

			template <typename T>
			Stream& operator >> (T& value)
			{
				if ((m_emOpenMode & OpenMode::eBinary) == 0)
				{
					m_file >> value;
				}
				else
				{
					m_file.read(reinterpret_cast<char*>(&value), sizeof(T));
				}
				return *this;
			}

			Stream& operator >> (std::string& value)
			{
				if ((m_emOpenMode & OpenMode::eBinary) == 0)
				{
					ReadLine(value);
				}
				else
				{
					uint32_t size = 0;
					*this >> size;

					if (size <= 0)
						return *this;

					value.resize(static_cast<size_t>(size));
					m_file.read(value.data(), size);

					value = value.substr(0, value.size() - 1);
				}

				return *this;
			}

		public:
			template <typename T>
			Stream& Write(const T* pValue, uint32_t nCount = 1)
			{
				for (uint32_t i = 0; i < nCount; ++i)
				{
					*this << pValue[i];
				}
				return *this;
			}

			Stream& Write(const char* pValue, size_t nLength)
			{
				if ((m_emOpenMode & OpenMode::eBinary) == 0)
				{
					m_file << pValue;
				}
				else
				{
					*this << nLength;
					m_file.write(pValue, nLength);
				}

				return *this;
			}

			template <typename T, size_t size>
			Stream& Write(const T(&value)[size])
			{
				for (uint32_t i = 0; i < size; ++i)
				{
					*this << value[i];
				}
				return *this;
			}

			template <typename T>
			Stream& Read(T* pBuffer, uint32_t nCount = 1)
			{
				for (uint32_t i = 0; i < nCount; ++i)
				{
					*this >> pBuffer[i];
				}
				return *this;
			}

			Stream& Read(char* pBuffer, size_t length)
			{
				m_file.read(pBuffer, length);
				return *this;
			}

			template <typename T, size_t size>
			Stream& Read(T(&value)[size])
			{
				for (uint32_t i = 0; i < size; ++i)
				{
					*this >> value[i];
				}
				return *this;
			}

			void ReadLine(std::string& str);

		private:
			std::fstream m_file;
			std::string m_path;
			OpenMode m_emOpenMode{ eNone };
			size_t m_fileSize{ 0 };

			std::vector<char> m_buffer;
		};
	}
}