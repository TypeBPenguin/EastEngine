#pragma once

#include "Singleton.h"

namespace EastEngine
{
	namespace Config
	{
		class SCommandLine : public Singleton<SCommandLine>
		{
			friend Singleton<SCommandLine>;
		private:
			SCommandLine();
			virtual ~SCommandLine();

		public:
			bool Init();
			void Release();

			bool GetCommandLine(const char* strCommandLine, std::string* pValue = nullptr);

		private:
			bool m_bInit;
			std::unordered_map<std::wstring, std::wstring> m_umapCommandLine;
		};
	}
}