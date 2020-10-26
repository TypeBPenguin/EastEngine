#include "stdafx.h"
#include "FileStream.h"

#include "FileUtil.h"
#include "StringUtil.h"

namespace est
{
	namespace file
	{
		static_assert(std::ios::in == eRead, "Openmode Mismatch");
		static_assert(std::ios::out == eWrite, "Openmode Mismatch");
		static_assert(std::ios::binary == eBinary, "Openmode Mismatch");

		Stream::Stream()
		{
		}

		Stream::~Stream()
		{
			Close();
		}

		bool Stream::Open(const wchar_t* fileName, OpenMode emMode)
		{
			m_emOpenMode = emMode;

			if (m_emOpenMode == OpenMode::eNone)
			{
				m_file.open(fileName);
			}
			else
			{
				m_file.open(fileName, m_emOpenMode);
			}

			if (m_file.fail())
				return false;

			if (m_file.bad())
				return false;

			m_fileSize = std::filesystem::file_size(fileName);

			m_path = fileName;

			return true;
		}

		void Stream::Close()
		{
			m_file.close();
		}

		bool Stream::Eof() const
		{
			return m_file.eof();
		}

		void Stream::Clear()
		{
			m_file.clear();
		}

		BinaryReader Stream::GetBinaryReader()
		{
			if (m_buffer.empty() == true)
			{
				m_buffer.resize(m_fileSize);
				Read(m_buffer.data(), m_fileSize);
			}

			return { m_buffer.data(), m_fileSize };
		}

		void Stream::ReadLine(std::string& str)
		{
			std::getline(m_file, str);
		}
	}
}