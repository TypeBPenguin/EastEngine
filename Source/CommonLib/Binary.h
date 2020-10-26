#pragma once

#include "Memory.h"
#include "StringUtil.h"

namespace est
{
	class BinaryWriter
	{
	public:
		BinaryWriter(size_t size)
			: m_buffer(size)
		{
		}
		virtual ~BinaryWriter() = default;

	public:
		virtual void* Allocate(size_t size)
		{
			assert(m_buffer.size() >= (m_position + size));

			const size_t curPosition = m_position;
			m_position += size;
			return &m_buffer[curPosition];
		}

		template <typename T>
		T* Allocate()
		{
			return reinterpret_cast<T*>(Allocate(sizeof(T)));
		}

		template <typename T>
		operator T&()
		{
			return *Allocate<T>();
		}

		template <typename T>
		BinaryWriter& operator << (const T& value)
		{
			Write(value);
			return *this;
		}

		template <const char*>
		BinaryWriter& operator << (const char* value)
		{
			WriteString(value, string::Length(value));
			return *this;
		}

		template <typename T>
		void Write(const T& value)
		{
			T* pDest = Allocate<T>();
			memory::Copy(pDest, &value, sizeof(T));
		}

		void WriteString(const char* value, uint32_t length)
		{
			uint32_t* pLength = Allocate<uint32_t>();
			*pLength = length + 1;
			
			char* pDest = static_cast<char*>(Allocate(*pLength));
			string::Copy(pDest, *pLength, value, *pLength);
		}

		template <uint32_t size>
		void WriteString(char(&source)[size])
		{
			WriteString(source, size);
		}

	private:
		std::vector<char> m_buffer;
		size_t m_position{ 0 };
	};

	class BinaryReader
	{
	public:
		BinaryReader(const char* binaryBuffer, size_t bufferSize)
			: m_pBuffer(binaryBuffer)
			, m_bufferSize(bufferSize)
		{
		}
		virtual ~BinaryReader() = default;

	public:
		const void* Read(size_t size)
		{
			assert(m_bufferSize >= (m_position + size));

			const size_t curPosition = m_position;
			m_position += size;
			return m_pBuffer + curPosition;
		}

		template <typename T>
		const T& Read()
		{
			return *reinterpret_cast<const T*>(Read(sizeof(T)));
		}

		template <typename T>
		const T& Read(size_t size)
		{
			return *reinterpret_cast<const T*>(Read(sizeof(T) * size));
		}

		const char* ReadString()
		{
			const uint32_t& length = Read<uint32_t>();
			return reinterpret_cast<const char*>(Read(length));
		}

		template <typename T>
		operator const T&()
		{
			return Read<T>();
		}

		operator const char*()
		{
			return ReadString();
		}

		template <typename T>
		BinaryReader& operator >> (T& value)
		{
			value = Read<T>();
		}

		BinaryReader& operator >> (const char*& value)
		{
			value = ReadString();
		}

	private:
		const char* m_pBuffer{ nullptr };
		const size_t m_bufferSize{ 0 };
		size_t m_position{ 0 };
	};
}