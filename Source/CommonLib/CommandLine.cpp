#include "stdafx.h"
#include "CommandLine.h"

#include "StringUtil.h"

#include <shellapi.h>

namespace eastengine
{
	namespace Config
	{
		SCommandLine::SCommandLine()
			: m_bInit(false)
		{
		}

		SCommandLine::~SCommandLine()
		{
			Release();
		}

		bool SCommandLine::Init()
		{
			if (m_bInit == true)
				return true;

			m_bInit = true;

			wchar_t* strCommandLine = GetCommandLineW();
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
			if (m_bInit == false)
				return;

			m_bInit = false;
		}

		bool SCommandLine::GetCommandLine(const char* strCommandLine, std::string* pValue)
		{
			if (m_bInit == false)
				return false;

			std::wstring wstrCommandLine = string::MultiToWide(strCommandLine);

			auto iter = m_umapCommandLine.find(wstrCommandLine);
			if (iter != m_umapCommandLine.end())
			{
				if (pValue != nullptr)
				{
					*pValue = string::WideToMulti(iter->second);
				}

				return true;
			}

			return false;
		}
	}
}