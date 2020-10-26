#include "stdafx.h"
#include "CommandLine.h"

#include "StringUtil.h"

#include <shellapi.h>

namespace est
{
	namespace Config
	{
		SCommandLine::SCommandLine()
			: m_isInitialize(false)
		{
		}

		SCommandLine::~SCommandLine()
		{
			Release();
		}

		bool SCommandLine::Init()
		{
			if (m_isInitialize == true)
				return true;

			m_isInitialize = true;

			wchar_t* strCommandLine = GetCommandLine();
			int numArgs = 0;
			wchar_t** argv = CommandLineToArgvW(strCommandLine, &numArgs);

			for (int i = 1; i < numArgs; i += 2)
			{
				m_umapCommandLine.insert(std::make_pair(argv[i], argv[i + 1]));
			}

			return true;
		}

		void SCommandLine::Release()
		{
			if (m_isInitialize == false)
				return;

			m_isInitialize = false;
		}

		bool SCommandLine::GetCommand(const wchar_t* commandLine, std::wstring* pValue)
		{
			if (m_isInitialize == false)
				return false;

			auto iter = m_umapCommandLine.find(commandLine);
			if (iter != m_umapCommandLine.end())
			{
				if (pValue != nullptr)
				{
					*pValue = iter->second;
				}

				return true;
			}

			return false;
		}
	}
}